#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

typedef int ImGuiKeyChord;

struct Window {
  std::string title;
  ImGuiKeyChord key;
  std::function<void()> cb;
  bool detached;
};

class UI {
private:
  std::vector<Window *> windows;
  std::unordered_map<std::string, ImGuiKeyChord> keys{
      {"toggle_ui", ImGuiKey_F10}, {"tool_player", ImGuiKey_F1},
      {"tool_map", ImGuiKey_F2},   {"tool_settings", ImGuiKey_F9},
      {"escape", ImGuiKey_Escape},
  };
  std::unordered_map<std::string, bool> options{
      {"visible", true}, {"tooltips", true}, {"automap", true},
      {"mouse", true},   {"block", true},
  };
  bool doWarp = false;
  bool inMenu = false;
  int lastMenuFrame = 0;
  int lastMinimapFrame = 0;

public:
  UI();
  ~UI();

  void Draw();
  bool Keys();
  void NewWindow(std::string title, ImGuiKeyChord key,
                 std::function<void()> cb);
  void Tooltip(std::string text);
  bool Option(std::string name);
  bool Block();
  void CreateMap();

  void DrawPlayer();
  void DrawMap();
  void DrawOptions();

  ID3D12Device *pD3DDevice = NULL;
  IDXGISwapChain3 *pSwapChain = NULL;
  ID3D12Resource *minimap_texture = NULL;
  D3D12_CPU_DESCRIPTOR_HANDLE minimap_srv_cpu_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE minimap_srv_gpu_handle;
  bool minimap_init = false;
  uint8_t minimap[800 * 528 * 4];
};
