#pragma once

#include <Windows.h>
#include <array>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <filesystem>
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

struct Sequencer {
  bool enabled{false};
  int base{4};
  int duration{8};
  int length{40};
  std::map<int, int> note;
  std::optional<uint8_t> a{0};
  std::optional<uint8_t> b{0};
  std::map<int, std::array<uint8_t, 200>> pages;
  int page{1};
  int page_loaded{1};
  int page_count{1};
};

struct SelectedTile {
  Tile *tile{nullptr};
  Coord room{0, 0};
  Coord pos{0, 0};
  int map{0};
};

struct SelectedRoom {
  Room *room{nullptr};
  Coord pos{0, 0};
  int map{0};
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
      {"toggle_lights", ImGuiMod_Shift | ImGuiKey_L},
      {"toggle_palette", ImGuiMod_Alt | ImGuiKey_L},
      {"toggle_player", ImGuiMod_Ctrl | ImGuiKey_B},
      {"toggle_gameboy", ImGuiMod_Ctrl | ImGuiKey_K},
      {"toggle_hud", ImGuiMod_Ctrl | ImGuiKey_H},
      {"toggle_mouse", ImGuiMod_Ctrl | ImGuiKey_N},
      {"warp", ImGuiMod_Ctrl | ImGuiKey_W},
      {"screenshot", ImGuiKey_Period},
      {"pause", ImGuiMod_Ctrl | ImGuiKey_Tab},
      {"skip", ImGuiKey_Tab},
      {"editor_modifier", ImGuiMod_Shift},
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
      {"cheat_lights",
       {false, "Remove lights",
        "Removes the lighting effect of hanging lights.", "toggle_lights"}},
      {"cheat_palette",
       {false, "Force color palette",
        "Forces a specific room color palette.\n(a very bright one by default)",
        "toggle_palette"}},
      {"cheat_gameboy",
       {false, "Gameboy mode",
        "Party like it's 1989! Enable the B&W death screen shader all the "
        "time.",
        "toggle_gameboy"}},
      {"cheat_hud",
       {false, "Hide ingame HUD",
        "Removes all HUD elements, including the menus.", "toggle_hud"}},
      {"cheat_player",
       {false, "Hide player character", "Doesn't render the player character.",
        "toggle_player"}},
      {"input_block",
       {true, "Block game input on UI input",
        "Blocks keyboard input from game\n"
        "when typing or menus are open."}},
      {"input_mouse",
       {true, "Mouse controls",
        "Left click to place tile, middle click to pick tile,\nright click to "
        "teleport!",
        "toggle_mouse"}},
      {"map_auto",
       {false, "Constantly update minimap",
        "This can be choppy if map window is left open.\n"
        "Default is to update map when it's opened."}},
      {"map_holes",
       {false, "Show destroyed tiles on minimap",
        "Shows what is already destroyed in pink."}},
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
      {"ui_grid",
       {false, "Show tile grid",
        "Draws a grid of 40 by 22.5 over the screen."}},
      {"ui_tooltips",
       {true, "Show helpful tooltips",
        "These are really helpful, why are you disabling them!?"}},
      {"ui_visible",
       {true, "Show UI", "Hide/show all MAXWELL windows.", "toggle_ui"}},
      {"ui_viewports",
       {false, "Multi-viewports",
        "Allow dragging UI windows outside the game window."}},
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
  std::string screenShotNextFrame = "";
  std::string screenShotThisFrame = "";
  Coord screenShotRange{1, 1};
  int screenShotIndex = -1;
  int screenShotFrame = -1;
  bool paused = false;
  Sequencer sequencer;
  SelectedTile selectedTile;
  SelectedRoom selectedRoom;
  Tile editorTile{0, 0, 0};
  std::string mapDir = "MAXWELL/Maps";
  std::vector<std::filesystem::path> maps;
  uint8_t forcedPalette{26};

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
  void DrawLevel();
  bool Button(std::string name, std::string desc = "", std::string key = "");
  void SaveINI();
  void LoadINI();
  void ScaleWindow();
  void SaveScreenShot(std::string name);
  void ScreenShot();
  void Play();
  void Cheats();
  void Windows();
  void HUD();
  void LoadMuralPage(int page);
  void RefreshMaps();

  HWND hWnd;
  ID3D12Device *pD3DDevice = NULL;
  IDXGISwapChain3 *pSwapChain = NULL;
  ID3D12Resource *minimap_texture = NULL;
  D3D12_CPU_DESCRIPTOR_HANDLE minimap_srv_cpu_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE minimap_srv_gpu_handle;
  bool minimap_init = false;
  uint8_t minimap[800 * 528 * 4];
};
