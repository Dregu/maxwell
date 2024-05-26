#define UNICODE

#include "hook.h"

#include <Windows.h>
#include <detours.h>

#include <atlbase.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <fstream>
#include <imgui.h>
#include <stdio.h>
#include <string>
#include <thread>

#include "font.h"
#include "logger.h"
#include "search.h"
#include "ui.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace D3D12 {

template <typename T> static void SafeRelease(T *&res) {
  if (res)
    res->Release();
  res = NULL;
}

// https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx12/main.cpp
struct FrameContext {
  CComPtr<ID3D12CommandAllocator> command_allocator = NULL;
  CComPtr<ID3D12Resource> main_render_target_resource = NULL;
  D3D12_CPU_DESCRIPTOR_HANDLE main_render_target_descriptor;
};

// Data
static std::vector<FrameContext> g_FrameContext;
static UINT g_FrameBufferCount = 0;

static CComPtr<ID3D12DescriptorHeap> g_pD3DRtvDescHeap = NULL;
static CComPtr<ID3D12DescriptorHeap> g_pD3DSrvDescHeap = NULL;
static CComPtr<ID3D12CommandQueue> g_pD3DCommandQueue = NULL;
static CComPtr<ID3D12GraphicsCommandList> g_pD3DCommandList = NULL;

LRESULT APIENTRY WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef long(__fastcall *Present)(IDXGISwapChain3 *pSwapChain,
                                  UINT SyncInterval, UINT Flags);
static Present OriginalPresent;

typedef void (*ExecuteCommandLists)(ID3D12CommandQueue *queue,
                                    UINT NumCommandLists,
                                    ID3D12CommandList *ppCommandLists);
static ExecuteCommandLists OriginalExecuteCommandLists;

typedef long(__fastcall *ResizeBuffers)(IDXGISwapChain3 *pSwapChain,
                                        UINT BufferCount, UINT Width,
                                        UINT Height, DXGI_FORMAT NewFormat,
                                        UINT SwapChainFlags);
static ResizeBuffers OriginalResizeBuffers;

static WNDPROC OriginalWndProc;
static HWND Window = nullptr;

static uint64_t *g_MethodsTable = NULL;
static bool g_Initialized = false;

static UI *g_UI;
static bool g_skipHook = false;

/*using GetKeyboardStatePtr = bool(PBYTE lpKeyState);
static GetKeyboardStatePtr *g_keyboard_trampoline{nullptr};
bool HookGetKeyboardState(PBYTE lpKeyState) {
  if (ImGui::GetIO().WantCaptureKeyboard)
    return true;
  return g_keyboard_trampoline(lpKeyState);
}*/

long __fastcall HookPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval,
                            UINT Flags) {
  if (g_pD3DCommandQueue == nullptr || g_skipHook) {
    return OriginalPresent(pSwapChain, SyncInterval, Flags);
  }
  if (!g_Initialized) {
    ID3D12Device *pD3DDevice;

    if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D12Device),
                                     (void **)&pD3DDevice))) {
      return OriginalPresent(pSwapChain, SyncInterval, Flags);
    }

    {
      DXGI_SWAP_CHAIN_DESC desc;
      pSwapChain->GetDesc(&desc);
      Window = desc.OutputWindow;
      g_UI->hWnd = Window;
      if (!OriginalWndProc) {
        OriginalWndProc = (WNDPROC)SetWindowLongPtr(
            Window, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
      }
      g_FrameBufferCount = desc.BufferCount;
      g_FrameContext.clear();
      g_FrameContext.resize(g_FrameBufferCount);
    }

    {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      desc.NumDescriptors = g_FrameBufferCount + 1;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

      if (pD3DDevice->CreateDescriptorHeap(
              &desc, IID_PPV_ARGS(&g_pD3DSrvDescHeap)) != S_OK) {
        return OriginalPresent(pSwapChain, SyncInterval, Flags);
      }
    }

    {
      D3D12_DESCRIPTOR_HEAP_DESC desc;
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      desc.NumDescriptors = g_FrameBufferCount + 1;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      desc.NodeMask = 1;

      if (pD3DDevice->CreateDescriptorHeap(
              &desc, IID_PPV_ARGS(&g_pD3DRtvDescHeap)) != S_OK) {
        return OriginalPresent(pSwapChain, SyncInterval, Flags);
      }

      const auto rtvDescriptorSize =
          pD3DDevice->GetDescriptorHandleIncrementSize(
              D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
          g_pD3DRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

      for (UINT i = 0; i < g_FrameBufferCount; i++) {

        g_FrameContext[i].main_render_target_descriptor = rtvHandle;
        pSwapChain->GetBuffer(
            i, IID_PPV_ARGS(&g_FrameContext[i].main_render_target_resource));
        pD3DDevice->CreateRenderTargetView(
            g_FrameContext[i].main_render_target_resource, nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
      }
    }

    {
      ID3D12CommandAllocator *allocator;
      if (pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                             IID_PPV_ARGS(&allocator)) !=
          S_OK) {
        return OriginalPresent(pSwapChain, SyncInterval, Flags);
      }

      for (size_t i = 0; i < g_FrameBufferCount; i++) {
        if (pD3DDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&g_FrameContext[i].command_allocator)) != S_OK) {
          return OriginalPresent(pSwapChain, SyncInterval, Flags);
        }
      }

      if (pD3DDevice->CreateCommandList(
              0, D3D12_COMMAND_LIST_TYPE_DIRECT,
              g_FrameContext[0].command_allocator, NULL,
              IID_PPV_ARGS(&g_pD3DCommandList)) != S_OK ||
          g_pD3DCommandList->Close() != S_OK) {
        return OriginalPresent(pSwapChain, SyncInterval, Flags);
      }

      {
        UINT handle_increment = pD3DDevice->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        int descriptor_index =
            1; // The descriptor table index to use (not normally a hard-coded
               // constant, but in this case we'll assume we have slot 1
               // reserved for us)
        g_UI->minimap_srv_cpu_handle =
            g_pD3DSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
        g_UI->minimap_srv_cpu_handle.ptr +=
            (handle_increment * descriptor_index);
        g_UI->minimap_srv_gpu_handle =
            g_pD3DSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
        g_UI->minimap_srv_gpu_handle.ptr +=
            (handle_increment * descriptor_index);
        g_UI->pD3DDevice = pD3DDevice;
        g_UI->pSwapChain = pSwapChain;
      }
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = "MAXWELL.ini";
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // TODO: doesn't work

    io.Fonts->AddFontFromMemoryCompressedTTF(OLFont_compressed_data,
                                             OLFont_compressed_size, 14.0f);

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.DisplaySafeAreaPadding = {0, 0};
    style.WindowPadding = {4, 4};
    style.WindowRounding = 0;
    style.FrameRounding = 0;
    style.PopupRounding = 0;
    style.GrabRounding = 0;
    style.TabRounding = 0;
    style.ScrollbarRounding = 0;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(Window);
    ImGui_ImplDX12_Init(
        pD3DDevice, g_FrameBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM,
        g_pD3DSrvDescHeap,
        g_pD3DSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pD3DSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    g_Initialized = true;

    pD3DDevice->Release();
  }

  ImGui_ImplWin32_NewFrame();
  ImGui_ImplDX12_NewFrame();
  ImGui::NewFrame();

  g_UI->Draw();

  FrameContext &currentFrameContext =
      g_FrameContext[pSwapChain->GetCurrentBackBufferIndex()];
  currentFrameContext.command_allocator->Reset();

  D3D12_RESOURCE_BARRIER barrier;
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource =
      currentFrameContext.main_render_target_resource;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  g_pD3DCommandList->Reset(currentFrameContext.command_allocator, nullptr);
  g_pD3DCommandList->ResourceBarrier(1, &barrier);
  g_pD3DCommandList->OMSetRenderTargets(
      1, &currentFrameContext.main_render_target_descriptor, FALSE, nullptr);
  g_pD3DCommandList->SetDescriptorHeaps(1, &g_pD3DSrvDescHeap);
  ImGui::Render();

  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pD3DCommandList);
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  g_pD3DCommandList->ResourceBarrier(1, &barrier);
  g_pD3DCommandList->Close();

  g_pD3DCommandQueue->ExecuteCommandLists(
      1, (ID3D12CommandList **)&g_pD3DCommandList);

  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    g_skipHook = true;
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault(nullptr, (void *)g_pD3DCommandList);
    g_skipHook = false;
  }

  return OriginalPresent(pSwapChain, SyncInterval, Flags);
}

void HookExecuteCommandLists(ID3D12CommandQueue *queue, UINT NumCommandLists,
                             ID3D12CommandList *ppCommandLists) {
  if (!g_pD3DCommandQueue &&
      queue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
    g_pD3DCommandQueue = queue;
  }

  OriginalExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}

void ResetState() {
  if (g_Initialized) {
    g_Initialized = false;
    ImGui_ImplWin32_Shutdown();
    ImGui_ImplDX12_Shutdown();
  }
  g_pD3DCommandQueue = nullptr;
  g_FrameContext.clear();
  g_pD3DCommandList = nullptr;
  g_pD3DRtvDescHeap = nullptr;
  g_pD3DSrvDescHeap = nullptr;
  g_UI->minimap_init = false;
}

long HookResizeBuffers(IDXGISwapChain3 *pSwapChain, UINT BufferCount,
                       UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                       UINT SwapChainFlags) {
  ResetState();
  return OriginalResizeBuffers(pSwapChain, BufferCount, Width, Height,
                               NewFormat, SwapChainFlags);
}

Status Init() {
  WNDCLASSEX windowClass;
  windowClass.cbSize = sizeof(WNDCLASSEX);
  windowClass.style = CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc = DefWindowProc;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = 0;
  windowClass.hInstance = GetModuleHandle(NULL);
  windowClass.hIcon = NULL;
  windowClass.hCursor = NULL;
  windowClass.hbrBackground = NULL;
  windowClass.lpszMenuName = NULL;
  windowClass.lpszClassName = L"Fake Window";
  windowClass.hIconSm = NULL;

  ::RegisterClassEx(&windowClass);

  HWND window = ::CreateWindow(
      windowClass.lpszClassName, L"Fake DirectX Window", WS_OVERLAPPEDWINDOW, 0,
      0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

  HMODULE libDXGI;
  HMODULE libD3D12;

  if ((libDXGI = ::GetModuleHandle(L"dxgi.dll")) == NULL) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::ModuleNotFoundError;
  }

  if ((libD3D12 = ::GetModuleHandle(L"d3d12.dll")) == NULL) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::ModuleNotFoundError;
  }

  void *CreateDXGIFactory;
  if ((CreateDXGIFactory = ::GetProcAddress(libDXGI, "CreateDXGIFactory")) ==
      NULL) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  CComPtr<IDXGIFactory> factory;
  if (((long(__stdcall *)(const IID &, void **))(CreateDXGIFactory))(
          __uuidof(IDXGIFactory), (void **)&factory) < 0) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  CComPtr<IDXGIAdapter> adapter;
  if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  void *D3D12CreateDevice;
  if ((D3D12CreateDevice = ::GetProcAddress(libD3D12, "D3D12CreateDevice")) ==
      NULL) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  CComPtr<ID3D12Device> device;
  if (((long(__stdcall *)(IUnknown *, D3D_FEATURE_LEVEL, const IID &, void **))(
          D3D12CreateDevice))(adapter, D3D_FEATURE_LEVEL_11_0,
                              __uuidof(ID3D12Device), (void **)&device) < 0) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  D3D12_COMMAND_QUEUE_DESC queueDesc;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Priority = 0;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.NodeMask = 0;

  CComPtr<ID3D12CommandQueue> commandQueue;
  if (device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue),
                                 (void **)&commandQueue) < 0) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  CComPtr<ID3D12CommandAllocator> commandAllocator;
  if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     __uuidof(ID3D12CommandAllocator),
                                     (void **)&commandAllocator) < 0) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  CComPtr<ID3D12GraphicsCommandList> commandList;
  if (device->CreateCommandList(
          0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL,
          __uuidof(ID3D12GraphicsCommandList), (void **)&commandList) < 0) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  DXGI_RATIONAL refreshRate;
  refreshRate.Numerator = 60;
  refreshRate.Denominator = 1;

  DXGI_MODE_DESC bufferDesc;
  bufferDesc.Width = 100;
  bufferDesc.Height = 100;
  bufferDesc.RefreshRate = refreshRate;
  bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

  DXGI_SAMPLE_DESC sampleDesc;
  sampleDesc.Count = 1;
  sampleDesc.Quality = 0;

  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  swapChainDesc.BufferDesc = bufferDesc;
  swapChainDesc.SampleDesc = sampleDesc;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.OutputWindow = window;
  swapChainDesc.Windowed = 1;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  CComPtr<IDXGISwapChain> swapChain;
  if (factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain) < 0) {
    ::DestroyWindow(window);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return Status::UnknownError;
  }

  g_MethodsTable = (uint64_t *)::calloc(150, sizeof(uint64_t));
  ::memcpy(g_MethodsTable, *(uint64_t **)(void *)device, 44 * sizeof(uint64_t));
  ::memcpy(g_MethodsTable + 44, *(uint64_t **)(void *)commandQueue,
           19 * sizeof(uint64_t));
  ::memcpy(g_MethodsTable + 44 + 19, *(uint64_t **)(void *)commandAllocator,
           9 * sizeof(uint64_t));
  ::memcpy(g_MethodsTable + 44 + 19 + 9, *(uint64_t **)(void *)commandList,
           60 * sizeof(uint64_t));
  ::memcpy(g_MethodsTable + 44 + 19 + 9 + 60, *(uint64_t **)(void *)swapChain,
           18 * sizeof(uint64_t));

  ::DestroyWindow(window);
  ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
  return Status::Success;
}

Status Hook(uint16_t _index, void **_original, void *_function) {
  void *target = (void *)g_MethodsTable[_index];
  *_original = target;
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach(&(PVOID &)*_original, _function);
  DetourTransactionCommit();
  return Status::Success;
}

Status Unhook(uint16_t _index, void **_original, void *_function) {
  void *target = (void *)g_MethodsTable[_index];
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourDetach(&(PVOID &)*_original, _function);
  DetourTransactionCommit();
  return Status::Success;
}

LRESULT APIENTRY WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (g_Initialized &&
      ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;
  return CallWindowProc(OriginalWndProc, hWnd, msg, wParam, lParam);
}

Status InstallHooks() {
  DetourRestoreAfterWith();

  Hook(54, (void **)&OriginalExecuteCommandLists, HookExecuteCommandLists);
  Hook(140, (void **)&OriginalPresent, HookPresent);
  Hook(145, (void **)&OriginalResizeBuffers, HookResizeBuffers);

  /*{
    g_keyboard_trampoline = (GetKeyboardStatePtr *)get_address("keyboard"sv);
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach((void **)&g_keyboard_trampoline, HookGetKeyboardState);
    const LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
      DEBUG("Failed hooking GetKeyboardState: {}\n", error);
    }
  }*/

  g_UI = new UI();

  return Status::Success;
}

Status RemoveHooks() {
  Unhook(54, (void **)&OriginalExecuteCommandLists, HookExecuteCommandLists);
  Unhook(140, (void **)&OriginalPresent, HookPresent);
  Unhook(145, (void **)&OriginalResizeBuffers, HookResizeBuffers);

  /*{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((void **)&g_keyboard_trampoline, HookGetKeyboardState);
    const LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
      DEBUG("Failed unhooking GetKeyboardState: {}\n", error);
    }
  }*/

  if (Window && OriginalWndProc) {
    SetWindowLongPtr(Window, GWLP_WNDPROC,
                     (__int3264)(LONG_PTR)OriginalWndProc);
  }

  delete g_UI;
  ResetState();
  ImGui::DestroyContext();

  // wait for hooks to finish if in one. maybe not needed, but seemed more
  // stable after adding it.
  Sleep(1000);
  return Status::Success;
}

} // namespace D3D12
