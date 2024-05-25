#include <imgui.h>
#include <imgui_internal.h>

#include "../logging.h"
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

void DrawMap() { ImGui::Text("Minimap here"); }

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
  NewWindow("Warp", keys["tool_warp"], DrawWarp);
  NewWindow("Minimap", keys["tool_map"], DrawMap);
  NewWindow("Settings", keys["tool_settings"],
            [this]() { this->DrawOptions(); });
  NewWindow("Style", ImGuiKey_None, []() { ImGui::ShowStyleEditor(); });
  LOG("MAXWELL UI INITIALIZED");
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
  ImGui::ShowDemoWindow();
}

void UI::NewWindow(std::string title, ImGuiKeyChord key,
                   std::function<void()> cb) {
  windows.push_back(new Window{title, key, cb});
}

void UI::Tooltip(std::string text) {
  if (options["tooltips"] && ImGui::IsItemHovered())
    ImGui::SetTooltip("Right click to detach a tool from the menu as window.");
}
