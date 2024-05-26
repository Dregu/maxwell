#include <array>
#include <imgui.h>
#include <imgui_internal.h>

#include "logger.h"
#include "max.h"
#include "memory.h"
#include "search.h"
#include "ui.h"
#include "version.h"

std::array equipment_names{
    "Unknown", "Firecrackers", "Flute ",   "Lantern ", "Top ",
    "Disc ",   "BWand ",       "Yoyo ",    "Slink ",   "Remote ",
    "Ball ",   "Wheel ",       "UVLight ",
};

std::array item_names{
    "MockDisc",  "SMedal",  "Cake",   "HouseKey",
    "OfficeKey", "CageKey", "EMedal", "FPack",
};

std::array misc_names{
    "HouseOpened",
    "OfficeOpened",
    "ClosetOpened",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",

    "SwitchState",
    "MapCollected",
    "StampsCollected",
    "PencilCollected",
    "ChameleonDefeated",
    "CRingCollected",
    "EatenByChameleon",
    "InsertedSMedal",

    "EMedalInserted",
    "WingsAcquired",
    "WokeUp",
    "BBWandUpgrade",
    "ManticoreEgg65",
    "AllCandlesLit",
    "SingularityActive",
    "ManticoreEggPlaced",

    "BatDefeated",
    "OstrichFreed",
    "OstrichDefeated",
    "EelFightActive",
    "EelDefeated",
    "NoDiscInShrine",
    "NoDiscInStatue",
    "Unknown",
};

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

template <std::size_t SIZE, typename T>
void Flags(const std::array<const char *, SIZE> names_array, T *flag_field,
           bool show_number = true) {
  for (int idx{0}; idx < SIZE && idx < sizeof(T) * 8; ++idx) {
    T value = (T)std::pow(2, idx);
    bool on = (*flag_field & value) == value;

    if (names_array[idx][0] != '\0' &&
        ImGui::Checkbox(
            show_number
                ? fmt::format("{}: {}", idx + 1, names_array[idx]).c_str()
                : names_array[idx],
            &on)) {
      *flag_field ^= value;
    }
  }
}

ImVec2 Normalize(ImVec2 pos) {
  ImGuiIO &io = ImGui::GetIO();
  ImVec2 res = io.DisplaySize;
  if (res.x / res.y > 1.78) {
    pos.x -= (res.x - res.y / 9 * 16) / 2;
    res.x = res.y / 9 * 16;
  } else if (res.x / res.y < 1.77) {
    pos.y -= (res.y - res.x / 16 * 9) / 2;
    res.y = res.x / 16 * 9;
  }
  return ImVec2(pos.x / res.x * 320.f, pos.y / res.y * 180.f);
}

void UI::DrawPlayer() {
  ImGui::PushItemWidth(120.f);
  ImGui::InputScalar("Slot", ImGuiDataType_U8, Max::get().slot_number(), NULL,
                     NULL, "%d", ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button("Save game")) {
    *Max::get().spawn_room() = *Max::get().player_room();
    Max::get().save_game();
  }
  if (ImGui::CollapsingHeader("Equipment"))
    Flags(equipment_names, Max::get().equipment(), false);
  if (ImGui::CollapsingHeader("Items"))
    Flags(item_names, Max::get().items(), false);
  if (ImGui::CollapsingHeader("Miscellaneous"))
    Flags(misc_names, Max::get().upgrades(), false);

  if (ImGui::CollapsingHeader("Consumables")) {
    ImGui::InputScalar("Health", ImGuiDataType_U8, Max::get().player_hp());
    ImGui::InputScalar("More health", ImGuiDataType_U8,
                       Max::get().player_hp() + 1);
    ImGui::InputScalar("Keys", ImGuiDataType_U8, Max::get().keys());
    ImGui::InputScalar("Matches", ImGuiDataType_U8, Max::get().keys() + 1);
    ImGui::InputScalar("Firecrackers", ImGuiDataType_U8, Max::get().keys() + 2);
  }
  if (ImGui::CollapsingHeader("Position")) {
    ImGui::InputInt2("Spawn", &Max::get().spawn_room()->x);
    ImGui::InputInt2("Room", &Max::get().player_room()->x);
    ImGui::InputFloat2("Position", &Max::get().player_position()->x);
    ImGui::InputFloat2("Velocity", &Max::get().player_velocity()->x);
    ImGui::InputInt("Layer", Max::get().player_layer());
    ImGui::InputScalar("State", ImGuiDataType_U8, Max::get().player_state());
    ImGui::InputScalar("Flute", ImGuiDataType_U8, Max::get().player_flute());
  }
  if (ImGui::CollapsingHeader("Warp")) {
    ImGui::InputInt2("Warp room", &Max::get().warp_room()->x);
    ImGui::InputInt2("Warp position", &Max::get().warp_position()->x);
    ImGui::InputInt("Warp layer", Max::get().warp_layer());
    if (ImGui::Button("Warp now"))
      doWarp = true;
    // TODO: Add saveable custom warp positions from current player/warp
    // position
  }
  ImGui::PopItemWidth();
}

// TODO: Add option to hide border rooms
void UI::DrawMap() {
  // ImGui::Text("CPU: %p", minimap_srv_cpu_handle);
  // ImGui::Text("GPU: %p", minimap_srv_gpu_handle);
  // ImGui::Text("TID: %p", minimap_texture);
  ImGuiIO &io = ImGui::GetIO();
  static const ImVec2 realmapsize{800, 528};
  ImVec2 bordersize{realmapsize.x / 20 * 2, realmapsize.y / 24 * 4};
  ImVec2 mapsize{realmapsize.x - bordersize.x * 2,
                 realmapsize.y - bordersize.y * 2};
  ImVec2 uv0{bordersize.x / realmapsize.x, bordersize.y / realmapsize.y};
  ImVec2 uv1{1 - uv0.x, 1 - uv0.y};
  if (!options["noborder"].value) {
    mapsize = realmapsize;
    bordersize = {0, 0};
    uv0 = {0, 0};
    uv1 = {1, 1};
  }
  static Coord cpos{0, 0};
  static Coord wroom{0, 0};
  static Coord wpos{0, 0};
  ImGui::PushItemWidth(0.3f * mapsize.x);
  ImGui::InputInt2("Room", &wroom.x);
  ImGui::SameLine(0.45f * mapsize.x);
  ImGui::InputInt2("Position", &wpos.x);
  ImGui::PopItemWidth();
  ImGui::SameLine(mapsize.x - 60.f);
  if (ImGui::Button("Update", ImVec2(60.f + ImGui::GetStyle().WindowPadding.x,
                                     ImGui::GetTextLineHeightWithSpacing())) ||
      (options["automap"].value &&
       ImGui::GetFrameCount() > lastMinimapFrame + 15) ||
      !minimap_init) {
    CreateMap();
    lastMinimapFrame = ImGui::GetFrameCount();
  }
  if (minimap_init) {
    auto a = ImGui::GetCursorPos();
    auto b = ImGui::GetMousePos();
    auto c = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
    auto d = ImGui::GetWindowPos();
    ImGui::PushStyleColor(ImGuiCol_Button, 0);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0);
    ImGui::ImageButton((ImTextureID)minimap_srv_gpu_handle.ptr, mapsize, uv0,
                       uv1, 0);
    Tooltip("Right click on the map to warp anywhere!");
    if (ImGui::IsItemHovered()) {
      cpos.x = (b.x - d.x) - a.x + c.x + bordersize.x;
      cpos.y = (b.y - d.y) - a.y + c.y + bordersize.y;
      wroom.x = cpos.x / realmapsize.x * 800 / 40;
      wroom.y = cpos.y / realmapsize.y * 528 / 22;
      wpos.x = ((int)(cpos.x / realmapsize.x * 800) % 40) * 8;
      wpos.y = ((int)(cpos.y / realmapsize.y * 528) % 22) * 8;
      if (io.MouseDown[1]) {
        *Max::get().player_state() = 18;
        *Max::get().warp_room() = wroom;
        *Max::get().warp_position() = wpos;
        doWarp = true;
      } else if (io.MouseReleased[1] && *Max::get().player_state() == 18) {
        *Max::get().player_state() = 0;
      }
    }
    ImGui::PopStyleColor(3);
    auto px = Max::get().player_room()->x * 40 +
              (Max::get().player_position()->x / 320.f * 40.f);
    auto py = Max::get().player_room()->y * 22 +
              (Max::get().player_position()->y / 180.f * 22.f);
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(a.x + d.x + px - c.x - bordersize.x,
               a.y + d.y + py - c.y - bordersize.y),
        4.f, 0xee0000ee);
  }
}

void UI::DrawOptions() {
  ImGuiIO &io = ImGui::GetIO();
  bool noclip = options["noclip"].value;
  for (auto &[name, enabled] : options) {
    Option(name);
  }
  if (noclip && !options["noclip"].value)
    *Max::get().player_state() = 0;
  if (ImGui::SliderInt("Window Scale", &windowScale, 1, 10)) {
    RECT c;
    RECT w;
    GetClientRect(hWnd, &c);
    GetWindowRect(hWnd, &w);
    int dx = (w.right - w.left) - (c.right - c.left);
    int dy = (w.bottom - w.top) - (c.bottom - c.top);
    SetWindowPos(hWnd, NULL, 0, 0, windowScale * 320 + dx,
                 windowScale * 180 + dy, 2);
  }
  // ImGui::InputFloat2("Display", &io.DisplaySize.x, "%.0f",
  // ImGuiInputTextFlags_ReadOnly);
  ImGui::SliderFloat("Alpha", &ImGui::GetStyle().Alpha, 0.2f, 1.0f, "%.1f");
}

bool UI::Option(std::string name) {
  std::string desc =
      options[name].name +
      (options[name].key != ""
           ? " (" +
                 std::string(ImGui::GetKeyChordName(keys[options[name].key])) +
                 ")"
           : "");
  if (inMenu)
    return ImGui::MenuItem(desc.c_str(), "", &options[name].value);
  return ImGui::Checkbox(desc.c_str(), &options[name].value);
}

UI::UI() {
  Max::get();

  NewWindow("Player", keys["tool_player"], 0, [this]() { this->DrawPlayer(); });
  NewWindow("Minimap", keys["tool_map"], ImGuiWindowFlags_AlwaysAutoResize,
            [this]() { this->DrawMap(); });
  NewWindow("Settings", keys["tool_settings"],
            ImGuiWindowFlags_AlwaysAutoResize,
            [this]() { this->DrawOptions(); });
  NewWindow("Debug", ImGuiKey_None, 0, [this]() {
    ImGuiIO &io = ImGui::GetIO();

    ImGui::Text("Check: %p", get_address("check"));
    ImGui::Text("State: %p", Max::get().state());
    ImGui::Text("Map: %p", Max::get().minimap());
    ImGui::Text("Slots: %p", get_address("slots"));
    ImGui::Text("Slot: %p", Max::get().slot());
    ImGui::Text("Layer: %p", get_address("layer_base"));
    ImGui::Text("Layer: %p", get_address("layer_offset"));
    if (!this->inMenu) {
      ImGui::ShowDemoWindow();
      ImGui::ShowMetricsWindow();
      if (ImGui::Begin("Styles")) {
        ImGui::ShowStyleEditor();
        ImGui::End();
      }
    }
  });
  DEBUG("MAXWELL UI INITIALIZED");
}

UI::~UI() {}

bool UI::Keys() {
  if (ImGui::IsKeyReleased((ImGuiKey)keys["escape"]))
    ImGui::SetWindowFocus(nullptr);
  else if (ImGui::IsKeyChordPressed(keys["toggle_ui"]))
    options["visible"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_noclip"])) {
    options["noclip"].value ^= true;
    if (!options["noclip"].value)
      *Max::get().player_state() = 0;
  } else if (ImGui::IsKeyChordPressed(keys["toggle_godmode"]))
    options["godmode"].value ^= true;
  else
    return false;
  return true;
}

void UI::Draw() {
  ImGuiIO &io = ImGui::GetIO();
  std::string version = fmt::format("MAXWELL {}", get_version());
  ImGui::GetBackgroundDrawList()->AddText(
      ImVec2(io.DisplaySize.x / 2.f -
                 ImGui::CalcTextSize(version.c_str()).x / 2.f,
             io.DisplaySize.y - ImGui::GetTextLineHeightWithSpacing()),
      0x99999999, version.c_str());
  doWarp = false;
  Keys();
  io.MouseDrawCursor = options["visible"].value;
  if (options["visible"].value) {
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
          lastMenuFrame = ImGui::GetFrameCount();
          ImGui::EndMenu();
        }
        Tooltip("Right click to detach a tool from the menu as window.");
        inMenu = false;
        if (io.MouseClicked[1] && ImGui::IsItemHovered())
          window->detached = true;
      }
      ImGui::EndMainMenuBar();
    }
    for (auto *window : windows) {
      if (!window->detached)
        continue;
      if (ImGui::Begin(window->title.c_str(), &window->detached,
                       window->flags)) {
        window->cb();
        ImGui::End();
      }
    }
  }

  {
    auto mpos = Normalize(io.MousePos);
    int x = mpos.x;
    int y = mpos.y;
    int rx = x / 8;
    int ry = y / 8;

    bool inbound = x > 0 && x < 320 && y > 0 && y < 180;

    if (options["mouse"].value && io.MouseDown[1] && !io.WantCaptureMouse &&
        ImGui::IsMousePosValid()) {
      if (inbound || (ImGui::GetFrameCount() % 10) == 0) {
        Max::get().player_position()->x = x - 4;
        Max::get().player_position()->y = y - 4;
      }
      Max::get().player_velocity()->x = 0;
      Max::get().player_velocity()->y = 0;
      *Max::get().player_state() = 18;
    } else if (io.MouseReleased[1] && *Max::get().player_state() == 18) {
      *Max::get().player_state() = 0;
    }
  }

  if (doWarp) {
    write_mem_recoverable("warp", get_address("warp"), "\xEB"sv, true);
  } else {
    recover_mem("warp");
  }

  if (options["block_input"].value) {
    if (Block()) {
      write_mem_recoverable("block", get_address("keyboard"), get_nop(6), true);
    } else {
      recover_mem("block");
    }
  }

  if (options["godmode"].value) {
    write_mem_recoverable("god", get_address("damage"), get_nop(6), true);
  } else {
    recover_mem("god");
  }

  if (options["noclip"].value) {
    *Max::get().player_state() = 18;
  }
}

void UI::NewWindow(std::string title, ImGuiKeyChord key, ImGuiWindowFlags flags,
                   std::function<void()> cb) {
  windows.push_back(new Window{title, key, flags, cb});
}

void UI::Tooltip(std::string text) {
  if (options["tooltips"].value && ImGui::IsItemHovered())
    ImGui::SetTooltip(text.c_str());
}

bool UI::Block() {
  ImGuiIO &io = ImGui::GetIO();
  return io.WantCaptureKeyboard || ImGui::GetFrameCount() < lastMenuFrame + 10;
}

void UI::CreateMap() {
  auto *raw_map = (void *)Max::get().minimap();
  if (raw_map == NULL)
    return;

  int image_width = 800;
  int image_height = 528;
  int length = image_width * image_height * 4;
  memcpy(minimap, raw_map, length);

  int i = 0;
  do {
    if (minimap[i + 3] == 0xf)
      minimap[i + 3] = 0xff;
    else
      minimap[i + 3] = 0x33;
    i += 4;
  } while (i < length);

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
           minimap + y * image_width * 4, image_width * 4);
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
