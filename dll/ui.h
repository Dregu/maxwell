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
  bool doWarp = false;
  bool inMenu = false;
  int lastMenuFrame = 0;
  int lastMinimapFrame = 0;
  std::chrono::system_clock::time_point lastMouseActivity =
      std::chrono::system_clock::now();
  ImVec2 lastMousePos = ImVec2(0, 0);
  float dpiScale = 1.0f;
  float uiScale = 1.0f;
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
  std::vector<std::filesystem::path> maps;
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
  void DrawMinimap();
  void DrawTools();
  void DrawOptions();
  void DrawAssets();
  void DrawMods();
  void DrawAsset(uint32_t id);
  void DumpAsset(uint32_t id);
  void DrawLevel();
  void DrawMap(uint8_t id);
  void DrawTile(Tile &tile);
  void DrawTileRow(Tile &tile);
  void DrawSelectedTile(SelectedTile &tile);
  void DrawSelectedTileRow(SelectedTile &tile, bool editable = true);
  void DrawCustomKey(std::string name, GAME_INPUT i);
  void DrawUIKeys();
  void KeyCapture();

  bool Button(std::string name, std::string desc = "", std::string key = "");
  bool SubMenu(std::string name);
  void EndMenu();
  void ScaleWindow();
  void SaveScreenShot(std::string name);
  void ScreenShot();
  void Play();
  void Cheats();
  void Windows();
  void HUD();
  void LoadMuralPage(int page);
  void RefreshMaps();

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
