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

#include "image.h"
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
  S32Vec2 room{0, 0};
  S32Vec2 pos{0, 0};
  int layer{0};
  int map{0};
};

struct SelectedRoom {
  Room *room{nullptr};
  S32Vec2 pos{0, 0};
  int map{0};
};

struct TargetTile {
  uint16_t tile;
  int n;
  int x;
  int y;
  int map;
};

ImVec2 Normalize(ImVec2 pos);

class UI {
private:
  std::vector<Window *> windows;
  std::map<std::string, ImGuiKeyChord> default_keys{
      {"escape", ImGuiKey_Escape},
      {"tool_player", ImGuiKey_F1},
      {"tool_map", ImGuiKey_F2},
      {"tool_tools", ImGuiKey_F3},
      {"tool_level", ImGuiKey_F4},
      {"toggle_ui", ImGuiKey_F10},
      {"toggle_noclip", ImGuiMod_Ctrl | ImGuiKey_F},
      {"toggle_godmode", ImGuiMod_Ctrl | ImGuiKey_G},
      {"toggle_damage", ImGuiMod_Shift | ImGuiKey_G},
      {"toggle_stats", ImGuiMod_Alt | ImGuiKey_G},
      {"toggle_darkness", ImGuiMod_Ctrl | ImGuiKey_L},
      {"toggle_lights", ImGuiMod_Shift | ImGuiKey_L},
      {"toggle_palette", ImGuiMod_Alt | ImGuiKey_L},
      {"toggle_clouds", ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_L},
      {"toggle_player", ImGuiMod_Ctrl | ImGuiKey_B},
      {"toggle_gameboy", ImGuiMod_Ctrl | ImGuiKey_K},
      {"toggle_hud", ImGuiMod_Ctrl | ImGuiKey_H},
      {"toggle_mouse", ImGuiMod_Ctrl | ImGuiKey_N},
      {"warp", ImGuiMod_Ctrl | ImGuiKey_W},
      {"screenshot", ImGuiKey_Period},
      {"pause", ImGuiKey_PageUp},
      {"skip", ImGuiKey_PageDown},
      {"submit_modifier", ImGuiMod_Ctrl},

      {"mouse_edit_fg", ImGuiKey_MouseLeft},
      {"mouse_edit_bg", ImGuiMod_Shift | ImGuiKey_MouseLeft},
      {"mouse_warp", ImGuiKey_MouseRight},
      {"mouse_select_fg", ImGuiKey_MouseMiddle},
      {"mouse_select_bg", ImGuiMod_Shift | ImGuiKey_MouseMiddle},
      {"mouse_destroy", ImGuiKey_MouseX1},
      {"mouse_fix", ImGuiMod_Shift | ImGuiKey_MouseX1},
      {"mouse_modifier", ImGuiMod_Shift},
  };
  std::map<std::string, ImGuiKeyChord> keys;
  // TODO: Save to ini
  std::map<std::string, Setting> options{
      {"cheat_active",
       {true, "Enable cheats",
        "Master switch to disable all cheats,\nignoring their current state."}},
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
       {false, "Disable darkness",
        "Removes the fog of war effect, see through walls.",
        "toggle_darkness"}},
      {"cheat_lights",
       {false, "Disable lamps",
        "Removes the lighting effect of hanging lights.", "toggle_lights"}},
      {"cheat_palette",
       {false, "Force lighting",
        "Forces a specific room color palette/lighting.\n(a very bright one by "
        "default)",
        "toggle_palette"}},
      {"cheat_gameboy",
       {false, "Gameboy mode",
        "Party like it's 1989! Enable the B&W death screen shader all the "
        "time.",
        "toggle_gameboy"}},
      {"cheat_groundhog",
       {false, "Disable Groundhog Day check",
        "Patches out the Groundhog Day checks,\nso you can always get up to 4 "
        "extra hearts."}},
      {"cheat_hud",
       {false, "Hide in-game HUD",
        "Removes all HUD elements, including the menus.", "toggle_hud"}},
      {"cheat_player",
       {false, "Hide player character", "Doesn't render the player character.",
        "toggle_player"}},
      {"cheat_clouds",
       {false, "Disable clouds", "Removes the cloudy foreground effects.",
        "toggle_clouds"}},
      {"cheat_credits",
       {false, "Skip credits", "Skips credits and drops the key immediately."}},
      {"cheat_igt",
       {false, "Use in-game time",
        "Replaces total game time with\nIGT that doesn't include paused "
        "time."}},
      {"cheat_water",
       {false, "Hide water", "Makes water see-through but still functional."}},
      {"cheat_stats",
       {false, "Max out consumables", "Keeps your consumables stacked.",
        "toggle_stats"}},

      {"input_block",
       {true, "Block game input on UI input",
        "Blocks keyboard input from game\n"
        "when typing or menus are open."}},
      {"input_custom",
       {false, "Use custom keyboard bindings",
        "Allows you to rebind all in-game\nkeys and disables default keys."}},
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
       {true, "Hide minimap margins",
        "There's nothing there really except some glitches."}},
      {"map_show",
       {false, "Show unseen minimap tiles in UI",
        "Shows what you haven't seen yet, but darker."}},
      {"map_reveal",
       {false, "Reveal unseen minimap tiles in game",
        "Marks all tiles seen on the actual in-game map."}},
      {"map_areas",
       {false, "Show all map borders on minimap",
        "Draws all special map borders instead of only selected layer."}},
      {"map_wheel", {false, "Show wheel on minimap", "For wheel warping."}},
      {"map_uv_bunny",
       {false, "Show UV bunny on minimap",
        "The UV bunny follows a path around the common areas next to the hub\n"
        "room, but turns around if it's about to run into your UV light."}},

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
       {true, "Show UI", "Hide all MAXWELL windows\nand disable UI input.",
        "toggle_ui"}},
      {"ui_ignore",
       {true, "Ignore UI input when hidden",
        "Ignore mouse and keyboard commands\nwhen UI is hidden with F10."}},
      {"ui_ignore_cheats",
       {false, "Disable cheats when hidden",
        "Disables all cheats temporarily\nwhen UI is hidden with F10."}},
      {"ui_viewports",
       {false, "Multi-viewports",
        "Allow dragging UI windows outside the game window."}},
      {"ui_scaling",
       {true, "DPI scaling",
        "Apply Windows UI scaling.\nMight require a restart to properly resize "
        "everything."}},
      {"ui_show_datetime", {true, "Show current time in UI"}},
      {"ui_show_cheats", {true, "Show enabled cheats in UI"}},
      {"ui_debug", {false, "Show debug information"}},
  };
  bool doWarp = false;
  bool inMenu = false;
  int lastMenuFrame = 0;
  int lastMinimapFrame = 0;
  std::chrono::system_clock::time_point lastMouseActivity =
      std::chrono::system_clock::now();
  ImVec2 lastMousePos = ImVec2(0, 0);
  int windowScale = 4;
  float dpiScale = 1.0f;
  float uiScale = 1.0f;
  float mapScale = 1.0f;
  std::string screenShotFileName = "MAXWELL";
  std::string screenShotNextFrame = "";
  std::string screenShotThisFrame = "";
  S32Vec2 screenShotRange{1, 1};
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
  std::vector<SelectedTile> searchTiles;
  std::unordered_map<Room *, RoomData> defaultRoom;
  std::unordered_map<uint8_t, LightingData> defaultLighting;
  std::string key_to_change = "";

public:
  UI(float scale = 1.0f);
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
  void UpdateOptions();
  void DrawLevel();
  void DrawTile(Tile &tile);
  void DrawTileRow(Tile &tile);
  void DrawSelectedTile(SelectedTile &tile);
  void DrawSelectedTileRow(SelectedTile &tile);
  void DrawCustomKey(std::string name, GAME_INPUT i);
  void DrawUIKeys();
  void KeyCapture();

  bool Button(std::string name, std::string desc = "", std::string key = "");
  bool SubMenu(std::string name);
  void EndMenu();
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

  bool GetOption(const std::string &name) { return options[name].value; }
  bool CheatsEnabled();
  template <std::size_t SIZE, typename T>
  int Flags(const std::array<const char *, SIZE> names_array, T *flag_field,
            bool show_number = false, int first = 0, bool go_button = false);
  template <typename T>
  int UnnamedFlags(const char *name, T *flag_field, int num, int offset = 0,
                   bool go_button = false);
  void WarpToTile(SelectedTile tile, int offsetx = 0, int offsety = 0);

  template <typename T> void DebugPtr(T *ptr);

  HWND hWnd;
  ID3D12Device *pD3DDevice = NULL;
  IDXGISwapChain3 *pSwapChain = NULL;
  ID3D12Resource *minimap_texture = NULL;
  D3D12_CPU_DESCRIPTOR_HANDLE minimap_srv_cpu_handle;
  D3D12_GPU_DESCRIPTOR_HANDLE minimap_srv_gpu_handle;
  bool minimap_init = false;
  uint8_t minimap[800 * 528 * 4];
};
