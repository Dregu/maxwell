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
  std::string desc;
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
  // TODO: Save to ini
  std::map<std::string, Setting> options{
      {"cheat_godmode",
       {false, "Godmode",
        "Disables taking any damage, but\nkeeps knockback, drowning etc.",
        "toggle_godmode"}},
      {"cheat_noclip",
       {false, "Noclip", "Do the cring thing without the cring.",
        "toggle_noclip"}},
      {"input_block",
       {true, "Block game input on UI input",
        "Blocks keyboard interaction from game\n"
        "when typing or menus are open."}},
      {"input_mouse",
       {true, "Right click to teleport",
        "Right click or drag yourself around!"}},
      {"map_auto",
       {false, "Auto-update minimap",
        "This can be choppy if map window is left open.\n"
        "Default is to update map when window is appearing."}},
      {"map_small",
       {true, "Hide minimap borders",
        "There's nothing there really except some glitches."}},
      {"map_reveal",
       {false, "Reveal unseen minimap tiles",
        "Shows what you haven't seen yet, but darker."}},
      {"ui_tooltips",
       {true, "Show helpful tooltips",
        "These are really helpful, why are you disabling them!?"}},
      {"ui_visible",
       {true, "Show UI",
        "Hide/show all MAXWELL windows."
        "toggle_ui"}},
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
