#pragma once

#include <Windows.h>
#include <array>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "max.h"

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
      {"escape", ImGuiKey_Escape},
      {"tool_player", ImGuiKey_F1},
      {"tool_map", ImGuiKey_F2},
      {"tool_tools", ImGuiKey_F3},
      {"toggle_ui", ImGuiKey_F10},
      {"toggle_noclip", ImGuiMod_Ctrl | ImGuiKey_F},
      {"toggle_godmode", ImGuiMod_Ctrl | ImGuiKey_G},
      {"toggle_damage", ImGuiMod_Ctrl | ImGuiKey_D},
      {"toggle_darkness", ImGuiMod_Ctrl | ImGuiKey_L},
      {"toggle_gameboy", ImGuiMod_Ctrl | ImGuiKey_K},
      {"toggle_hud", ImGuiMod_Ctrl | ImGuiKey_H},
      {"warp", ImGuiMod_Ctrl | ImGuiKey_W},
      {"screenshot", ImGuiKey_Period},
  };
  // TODO: Save to ini
  std::map<std::string, Setting> options{
      {"cheat_damage",
       {false, "No damage",
        "Disables taking any damage, but\nkeeps knockback, drowning etc.",
        "toggle_damage"}},
      {"cheat_godmode",
       {false, "God mode",
        "Disables taking any damage or knockback,\nsimply walk through most "
        "threats.",
        "toggle_godmode"}},
      {"cheat_noclip",
       {false, "Noclip", "Do the cring thing without the cring.",
        "toggle_noclip"}},
      {"cheat_darkness",
       {false, "Remove darkness",
        "Removes the fog of war effect, see through walls.",
        "toggle_darkness"}},
      {"cheat_gameboy",
       {false, "Gameboy mode",
        "Party like it's 1989! Enable the B&W death screen shader all the "
        "time.",
        "toggle_gameboy"}},
      {"cheat_hud",
       {false, "Hide ingame HUD",
        "Removes all HUD elements, including the menus.", "toggle_hud"}},
      {"input_block",
       {true, "Block game input on UI input",
        "Blocks keyboard input from game\n"
        "when typing or menus are open."}},
      {"input_mouse",
       {true, "Right click to teleport",
        "Right click or drag yourself around!"}},
      {"map_auto",
       {false, "Constantly update minimap",
        "This can be choppy if map window is left open.\n"
        "Default is to update map when it's opened."}},
      {"map_small",
       {true, "Hide minimap borders",
        "There's nothing there really except some glitches."}},
      {"map_show",
       {false, "Show unseen minimap tiles in UI",
        "Shows what you haven't seen yet, but darker."}},
      {"map_reveal",
       {false, "Reveal unseen minimap tiles in game",
        "Marks all tiles seen on the actual ingame map."}},
      {"map_areas",
       {false, "Show all special layers on minimap",
        "Draws all special map borders instead of only selected layer."}},
      {"map_wheel", {false, "Show wheel on minimap", "For wheel warping."}},
      {"ui_coords",
       {true, "Show coordinate tooltip",
        "Show screen and tile coordinates under mouse cursor."}},
      {"ui_hideplayer",
       {false, "Hide player from screenshots",
        "Moves the player offscreen for\nthe durating of screenshot."}},
      {"ui_tooltips",
       {true, "Show helpful tooltips",
        "These are really helpful, why are you disabling them!?"}},
      {"ui_visible",
       {true, "Show UI", "Hide/show all MAXWELL windows.", "toggle_ui"}},
  };
  bool doWarp = false;
  bool inMenu = false;
  int lastMenuFrame = 0;
  int lastMinimapFrame = 0;
  std::chrono::system_clock::time_point lastMouseActivity =
      std::chrono::system_clock::now();
  ImVec2 lastMousePos = ImVec2(0, 0);
  int windowScale = 4;
  std::string screenShotFileName = "MAXWELL";
  std::string screenShotNextFrame;
  std::string screenShotThisFrame;
  Coord screenShotPlayerRoom{-1, -1};
  Coord screenShotRange{1, 1};
  int screenShotIndex = -1;
  int screenShotFrame = -1;

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
  void DrawTools();
  void DrawOptions();
  bool Button(std::string name, std::string desc = "", std::string key = "");
  void SaveINI();
  void LoadINI();
  void ScaleWindow();
  void SaveScreenShot(std::string name);
  void ScreenShot();

  HWND hWnd;
  ID3D12Device *pD3DDevice = NULL;
  IDXGISwapChain3 *pSwapChain = NULL;
  ID3D12Resource *minimap_texture = NULL;
  D3D12_CPU_DESCRIPTOR_HANDLE minimap_srv_cpu_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE minimap_srv_gpu_handle;
  bool minimap_init = false;
  uint8_t minimap[800 * 528 * 4];
};
