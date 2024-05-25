#include <imgui.h>
#include <imgui_internal.h>

#include "logger.h"
#include "max.h"
#include "search.h"
#include "ui.h"

namespace ImGui {
// Wrapper for menu that can be opened with a global shortcut
// or submenu with a local shortcut
inline bool BeginMenu(const char *label, const ImGuiKeyChord key) {
  if (key != ImGuiKey_None && ImGui::IsKeyChordPressed(key))
    ImGui::OpenPopup(label);
  return ImGui::BeginMenu(label);
};
// Wrapper for menuitem that can be opened with a local shortcut
inline bool MenuItem(const char *label, const ImGuiKeyChord key) {
  char shortcut[32];
  ImGui::GetKeyChordName(key);
  return ImGui::MenuItem(label, shortcut) || ImGui::IsKeyChordPressed(key);
}
} // namespace ImGui

void DrawWarp() {
  ImGui::Text("Warping here");
  ImGui::Text("Warping here");
  ImGui::Text("Warping here");
  ImGui::Text("Warping here");
  ImGui::Text("Warping here");
}

void UI::DrawMap() {
  static Coord cpos{0, 0};
  static Coord wroom{0, 0};
  static Coord wpos{0, 0};
  ImGui::InputInt2("Warp room", &wroom.x);
  ImGui::InputInt2("Warp pos", &wpos.x);
  ImGui::Text("CPU: %p", minimap_srv_cpu_handle);
  ImGui::Text("GPU: %p", minimap_srv_gpu_handle);
  ImGui::Text("TID: %p", minimap_texture);
  if (ImGui::Button("Update map")) {
    CreateMap();
  }
  if (minimap_init) {
    static ImVec2 mapsize{800, 528};
    // static ImVec2 mapsize{ 1600, 1056 };
    auto a = ImGui::GetCursorPos();
    auto b = ImGui::GetMousePos();
    auto c = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
    auto d = ImGui::GetWindowPos();
    cpos.x = (b.x - d.x) - a.x + c.x;
    cpos.y = (b.y - d.y) - a.y + c.y;
    wroom.x = cpos.x / mapsize.x * 800 / 40;
    wroom.y = cpos.y / mapsize.y * 528 / 22;
    wpos.x = ((int)(cpos.x / mapsize.x * 800) % 40) * 8;
    wpos.y = ((int)(cpos.y / mapsize.y * 528) % 22) * 8;
    if (ImGui::ImageButton((ImTextureID)minimap_srv_gpu_handle.ptr, mapsize,
                           ImVec2(0, 0), ImVec2(1, 1), 0)) {
      // room->x = wroom.x;
      // room->y = wroom.y;
      // pos->x = wpos.x;
      // pos->y = wpos.y;
      // do_warp = true;
    }
  }
}

void UI::DrawOptions() {
  for (auto &[name, enabled] : options) {
    Option(name);
  }
}

bool UI::Option(std::string name) {
  if (inMenu)
    return ImGui::MenuItem(name.c_str(), "", &options[name]);
  return ImGui::Checkbox(name.c_str(), &options[name]);
}

UI::UI() {
  Max::get();

  NewWindow("Warp", keys["tool_warp"], DrawWarp);
  NewWindow("Minimap", keys["tool_map"], [this]() { this->DrawMap(); });
  NewWindow("Settings", keys["tool_settings"],
            [this]() { this->DrawOptions(); });
  NewWindow("Style", ImGuiKey_None, []() { ImGui::ShowStyleEditor(); });
  NewWindow("Debug", ImGuiKey_None, [this]() {
    ImGui::Text("State: %p", Max::get().state());
    ImGui::Text("Map: %p", Max::get().minimap());
    ImGui::Text("Slots: %p", get_address("slots"));
    ImGui::Text("Slot num: %d", Max::get().slot_number());
    ImGui::Text("Slot: %p", Max::get().slot());
    if (!this->inMenu) {
      ImGui::ShowDemoWindow();
      ImGui::ShowMetricsWindow();
    }
  });
  DEBUG("MAXWELL UI INITIALIZED");
}

UI::~UI() {}

bool UI::Keys() {
  if (ImGui::IsKeyChordPressed(keys["escape"]))
    ImGui::SetWindowFocus(nullptr);
  else if (ImGui::IsKeyChordPressed(keys["toggle_ui"]))
    options["visible"] = !options["visible"];
  else
    return false;
  return true;
}

void UI::Draw() {
  Keys();
  ImGuiIO &io = ImGui::GetIO();
  io.MouseDrawCursor = options["visible"];
  if (!options["visible"])
    return;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  if (ImGui::BeginMainMenuBar()) {
    ImGui::PopStyleVar(2);
    for (auto *window : windows) {
      if (window->detached)
        continue;
      inMenu = true;
      if (ImGui::BeginMenu(window->title.c_str(), window->key)) {
        window->cb();
        ImGui::EndMenu();
      }
      inMenu = false;
      Tooltip("Right click to detach a tool from the menu as window.");
      if (io.MouseClicked[1] && ImGui::IsItemHovered())
        window->detached = true;
    }
    ImGui::EndMainMenuBar();
  }
  for (auto *window : windows) {
    if (!window->detached)
      continue;
    if (ImGui::Begin(window->title.c_str(), &window->detached)) {
      window->cb();
      ImGui::End();
    }
  }
}

void UI::NewWindow(std::string title, ImGuiKeyChord key,
                   std::function<void()> cb) {
  windows.push_back(new Window{title, key, cb});
}

void UI::Tooltip(std::string text) {
  if (options["tooltips"] && ImGui::IsItemHovered())
    ImGui::SetTooltip("Right click to detach a tool from the menu as window.");
}

bool UI::Block() {
  ImGuiIO &io = ImGui::GetIO();
  return io.WantCaptureKeyboard;
}

void UI::CreateMap() {
  static size_t minimap = Max::get().minimap();
  if (minimap == NULL)
    return;

  int image_width = 800;
  int image_height = 528;
  unsigned char *image_data = (uint8_t *)minimap;
  // unsigned char image_data[800 * 528 * 4];
  // memcpy(image_data, (uint8_t*)minimap, 800 * 528 * 4);

  int i = 0;
  do {
    // image_data[i] = minimap[i];
    // image_data[i+1] = minimap[i+1];
    // image_data[i+2] = minimap[i+2];
    // image_data[i+3] = 0xff;
    ((uint8_t *)minimap)[i + 3] = 0xff;
    i += 4;
  } while (i < 800 * 528 * 4);

  auto d3d_device = pD3DDevice;

  // Create texture resource
  D3D12_HEAP_PROPERTIES props;
  memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
  props.Type = D3D12_HEAP_TYPE_DEFAULT;
  props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

  D3D12_RESOURCE_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  desc.Alignment = 0;
  desc.Width = image_width;
  desc.Height = image_height;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  ID3D12Resource *pTexture = NULL;
  d3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
                                      D3D12_RESOURCE_STATE_COPY_DEST, NULL,
                                      IID_PPV_ARGS(&pTexture));

  // Create a temporary upload resource to move the data in
  UINT uploadPitch =
      (image_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) &
      ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
  UINT uploadSize = image_height * uploadPitch;
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Alignment = 0;
  desc.Width = uploadSize;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  props.Type = D3D12_HEAP_TYPE_UPLOAD;
  props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

  ID3D12Resource *uploadBuffer = NULL;
  HRESULT hr = d3d_device->CreateCommittedResource(
      &props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
      NULL, IID_PPV_ARGS(&uploadBuffer));
  IM_ASSERT(SUCCEEDED(hr));

  // Write pixels into the upload resource
  void *mapped = NULL;
  D3D12_RANGE range = {0, uploadSize};
  hr = uploadBuffer->Map(0, &range, &mapped);
  IM_ASSERT(SUCCEEDED(hr));
  for (int y = 0; y < image_height; y++)
    memcpy((void *)((uintptr_t)mapped + y * uploadPitch),
           image_data + y * image_width * 4, image_width * 4);
  uploadBuffer->Unmap(0, &range);

  // Copy the upload resource content into the real resource
  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource = uploadBuffer;
  srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srcLocation.PlacedFootprint.Footprint.Width = image_width;
  srcLocation.PlacedFootprint.Footprint.Height = image_height;
  srcLocation.PlacedFootprint.Footprint.Depth = 1;
  srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource = pTexture;
  dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dstLocation.SubresourceIndex = 0;

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = pTexture;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

  // Create a temporary command queue to do the copy with
  ID3D12Fence *fence = NULL;
  hr = d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
  IM_ASSERT(SUCCEEDED(hr));

  HANDLE event = CreateEvent(0, 0, 0, 0);
  IM_ASSERT(event != NULL);

  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.NodeMask = 1;

  ID3D12CommandQueue *cmdQueue = NULL;
  hr = d3d_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
  IM_ASSERT(SUCCEEDED(hr));

  ID3D12CommandAllocator *cmdAlloc = NULL;
  hr = d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                          IID_PPV_ARGS(&cmdAlloc));
  IM_ASSERT(SUCCEEDED(hr));

  ID3D12GraphicsCommandList *cmdList = NULL;
  hr = d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     cmdAlloc, NULL, IID_PPV_ARGS(&cmdList));
  IM_ASSERT(SUCCEEDED(hr));

  cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
  cmdList->ResourceBarrier(1, &barrier);

  hr = cmdList->Close();
  IM_ASSERT(SUCCEEDED(hr));

  // Execute the copy
  cmdQueue->ExecuteCommandLists(1, (ID3D12CommandList *const *)&cmdList);
  hr = cmdQueue->Signal(fence, 1);
  IM_ASSERT(SUCCEEDED(hr));

  // Wait for everything to complete
  fence->SetEventOnCompletion(1, event);
  WaitForSingleObject(event, INFINITE);

  // Tear down our temporary command queue and release the upload resource
  cmdList->Release();
  cmdAlloc->Release();
  cmdQueue->Release();
  CloseHandle(event);
  fence->Release();
  uploadBuffer->Release();

  // Create a shader resource view for the texture
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
  ZeroMemory(&srvDesc, sizeof(srvDesc));
  srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = desc.MipLevels;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  d3d_device->CreateShaderResourceView(pTexture, &srvDesc,
                                       minimap_srv_cpu_handle);

  // Return results
  minimap_texture = pTexture;
  minimap_init = true;
}
