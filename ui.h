#pragma once

#include <Windows.h>
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
      {"toggle_ui", ImGuiKey_F10}, {"tool_warp", ImGuiKey_F1},
      {"tool_map", ImGuiKey_F2},   {"tool_settings", ImGuiKey_F9},
      {"escape", ImGuiKey_Escape},
  };
  std::unordered_map<std::string, bool> options{
      {"visible", true},
      {"tooltips", true},
  };
  bool inMenu = false;

public:
  UI();
  ~UI();

  void Draw();
  bool Keys();
  void NewWindow(std::string title, ImGuiKeyChord key,
                 std::function<void()> cb);
  void Tooltip(std::string text);
  void DrawOptions();
  bool Option(std::string name);
};
