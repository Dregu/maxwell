#pragma once

#include <Windows.h>
#include <array>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

typedef int ImGuiKeyChord;

struct Window {
  std::string title;
  ImGuiKeyChord key;
  ImGuiWindowFlags flags;
  std::function<void()> cb;
  bool detached;
};

struct Setting {
  bool value;
  std::string name;
  std::string key;
};

ImVec2 Normalize(ImVec2 pos);

class UI {
private:
  std::vector<Window *> windows;
  std::map<std::string, ImGuiKeyChord> keys{
      {"toggle_ui", ImGuiKey_F10},
      {"tool_player", ImGuiKey_F1},
      {"tool_map", ImGuiKey_F2},
      {"tool_settings", ImGuiKey_F9},
      {"escape", ImGuiKey_Escape},
      {"toggle_noclip", ImGuiMod_Ctrl | ImGuiKey_F},
      {"toggle_godmode", ImGuiMod_Ctrl | ImGuiKey_G},
  };
  std::map<std::string, Setting> options{
      {"visible", {true, "Show UI", "toggle_ui"}},
      {"tooltips", {true, "Show tooltips"}},
      {"automap", {true, "Auto-update minimap"}},
      {"mouse", {true, "Right click teleport"}},
      {"block_input", {true, "Block game input on text input"}},
      {"noclip", {false, "Noclip", "toggle_noclip"}},
      {"godmode", {false, "Godmode", "toggle_godmode"}},
  };
  bool doWarp = false;
  bool inMenu = false;
  int lastMenuFrame = 0;
  int lastMinimapFrame = 0;
  int windowScale = 4;

public:
  UI();
  ~UI();

  void Draw();
  bool Keys();
  void NewWindow(std::string title, ImGuiKeyChord key, ImGuiWindowFlags flags,
                 std::function<void()> cb);
  void Tooltip(std::string text);
  bool Option(std::string name);
  bool Block();
  void CreateMap();

  void DrawPlayer();
  void DrawMap();
  void DrawOptions();

  HWND hWnd;
  ID3D12Device *pD3DDevice = NULL;
  IDXGISwapChain3 *pSwapChain = NULL;
  ID3D12Resource *minimap_texture = NULL;
  D3D12_CPU_DESCRIPTOR_HANDLE minimap_srv_cpu_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE minimap_srv_gpu_handle;
  bool minimap_init = false;
  uint8_t minimap[800 * 528 * 4];
};
