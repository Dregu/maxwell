#define UNICODE
#define IMGUI_DEFINE_MATH_OPERATORS

#include <Windows.h>

#include <array>
#include <chrono>
#include <d3d12.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <map>
#include <misc/cpp/imgui_stdlib.h>
#include <shellapi.h>
#include <toml.hpp>

#include "ghidra_byte_string.h"
#include "logger.h"
#include "memory.h"
#include "search.h"
#include "ui.h"
#include "version.h"

#pragma comment(lib, "version.lib")

const char s8_zero = 0, s8_one = 1, s8_fifty = 50, s8_min = -128, s8_max = 127;
const ImU8 u8_zero = 0, u8_one = 1, u8_fifty = 50, u8_min = 0, u8_max = 255;
const short s16_zero = 0, s16_one = 1, s16_fifty = 50, s16_min = -32768,
            s16_max = 32767;
const ImU16 u16_zero = 0, u16_one = 1, u16_fifty = 50, u16_min = 0,
            u16_max = 65535;
const ImS32 s32_zero = 0, s32_one = 1, s32_fifty = 50, s32_min = INT_MIN / 2,
            s32_max = INT_MAX / 2, s32_hi_a = INT_MAX / 2 - 100,
            s32_hi_b = INT_MAX / 2;
const ImU32 u32_zero = 0, u32_one = 1, u32_fifty = 50, u32_min = 0,
            u32_max = UINT_MAX / 2, u32_hi_a = UINT_MAX / 2 - 100,
            u32_hi_b = UINT_MAX / 2;
const ImS64 s64_zero = 0, s64_one = 1, s64_fifty = 50, s64_min = LLONG_MIN / 2,
            s64_max = LLONG_MAX / 2, s64_hi_a = LLONG_MAX / 2 - 100,
            s64_hi_b = LLONG_MAX / 2;
const ImU64 u64_zero = 0, u64_one = 1, u64_fifty = 50, u64_min = 0,
            u64_max = ULLONG_MAX / 2, u64_hi_a = ULLONG_MAX / 2 - 100,
            u64_hi_b = ULLONG_MAX / 2;
const float f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f,
            f32_hi_a = +10000000000.0f;
const double f64_zero = 0., f64_one = 1., f64_lo_a = -1000000000000000.0,
             f64_hi_a = +1000000000000000.0;

std::array equipment_names{
    "Unknown", "Firecrackers", "Flute",  "Lantern", "Top",   "Disc",    "BWand",
    "Yoyo",    "Slink",        "Remote", "Ball",    "Wheel", "UVLight",
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

const std::map<std::string, PLAYER_INPUT> notes{
    {"A4", (PLAYER_INPUT)(PLAYER_INPUT::RIGHT | PLAYER_INPUT::LB)},
    {"A#4",
     (PLAYER_INPUT)(PLAYER_INPUT::RIGHT | PLAYER_INPUT::LB | PLAYER_INPUT::RB)},
    {"B4", (PLAYER_INPUT)(PLAYER_INPUT::RIGHT | PLAYER_INPUT::DOWN |
                          PLAYER_INPUT::LB)},
    {"C5", (PLAYER_INPUT)(PLAYER_INPUT::RIGHT | PLAYER_INPUT::DOWN |
                          PLAYER_INPUT::LB | PLAYER_INPUT::RB)},
    {"C#5", (PLAYER_INPUT)(PLAYER_INPUT::DOWN | PLAYER_INPUT::LB)},
    {"D5", (PLAYER_INPUT)(PLAYER_INPUT::DOWN | PLAYER_INPUT::LEFT |
                          PLAYER_INPUT::LB)},
    {"D#5", (PLAYER_INPUT)(PLAYER_INPUT::DOWN | PLAYER_INPUT::LEFT |
                           PLAYER_INPUT::LB | PLAYER_INPUT::RB)},
    {"E5", (PLAYER_INPUT)(PLAYER_INPUT::LEFT | PLAYER_INPUT::LB)},
    {"F5",
     (PLAYER_INPUT)(PLAYER_INPUT::LEFT | PLAYER_INPUT::LB | PLAYER_INPUT::RB)},
    {"F#5",
     (PLAYER_INPUT)(PLAYER_INPUT::UP | PLAYER_INPUT::LEFT | PLAYER_INPUT::LB)},
    {"G5", (PLAYER_INPUT)(PLAYER_INPUT::UP | PLAYER_INPUT::LEFT |
                          PLAYER_INPUT::LB | PLAYER_INPUT::RB)},
    {"G#5", (PLAYER_INPUT)(PLAYER_INPUT::UP | PLAYER_INPUT::LB)},
    {"A5", (PLAYER_INPUT)(PLAYER_INPUT::RIGHT)},
    {"A#5", (PLAYER_INPUT)(PLAYER_INPUT::RIGHT | PLAYER_INPUT::RB)},
    {"B5", (PLAYER_INPUT)(PLAYER_INPUT::RIGHT | PLAYER_INPUT::DOWN)},
    {"C6", (PLAYER_INPUT)(PLAYER_INPUT::RIGHT | PLAYER_INPUT::DOWN |
                          PLAYER_INPUT::RB)},
    {"C#6", (PLAYER_INPUT)(PLAYER_INPUT::DOWN)},
    {"D6", (PLAYER_INPUT)(PLAYER_INPUT::DOWN | PLAYER_INPUT::LEFT)},
    {"D#6", (PLAYER_INPUT)(PLAYER_INPUT::DOWN | PLAYER_INPUT::LEFT |
                           PLAYER_INPUT::RB)},
    {"E6", (PLAYER_INPUT)(PLAYER_INPUT::LEFT)},
    {"F6", (PLAYER_INPUT)(PLAYER_INPUT::LEFT | PLAYER_INPUT::RB)},
    {"F#6", (PLAYER_INPUT)(PLAYER_INPUT::LEFT | PLAYER_INPUT::UP)},
    {"G6",
     (PLAYER_INPUT)(PLAYER_INPUT::LEFT | PLAYER_INPUT::UP | PLAYER_INPUT::RB)},
    {"G#6", (PLAYER_INPUT)(PLAYER_INPUT::UP)},
    {"A6", (PLAYER_INPUT)(PLAYER_INPUT::UP | PLAYER_INPUT::RIGHT)},
    {"A#6",
     (PLAYER_INPUT)(PLAYER_INPUT::UP | PLAYER_INPUT::RIGHT | PLAYER_INPUT::RB)},
};

std::array note_order{"A4",  "A#4", "B4",  "C5", "C#5", "D5", "D#5",
                      "E5",  "F5",  "F#5", "G5", "G#5", "A5", "A#5",
                      "B5",  "C6",  "C#6", "D6", "D#6", "E6", "F6",
                      "F#6", "G6",  "G#6", "A6", "A#6"};

std::array ttfaf{
    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A4",  "G5",  "A4",  "G#5", "A4",
    "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "D5",  "A4",

    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A4",  "G5",  "A4",  "G#5", "A4",
    "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "D5",  "A4",

    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A4",  "G5",  "A4",  "G#5", "A4",
    "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "D5",  "A4",

    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A4",  "G5",  "A4",  "G#5", "A4",
    "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "D5",  "A4",

    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A4",  "G5",  "A4",  "G#5", "A4",
    "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "D5",  "A4",

    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A4",  "G5",  "A4",  "G#5", "A4",
    "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "D5",  "A4",

    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A4",  "G5",  "A4",  "G#5", "A4",
    "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "D5",  "A4",

    "D#5", "A4",  "F5",  "A4",  "G5",  "A4",  "D#5", "A4",  "F5",  "A4",  "G5",
    "A4",  "G#5", "A4",  "F5",  "A4",  "A#5", "A5",  "D#5", "C5",  "A4",  "C5",
    "D#5", "G5",  "A#5", "C6",  "A#5", "G5",  "D#5", "A4",  "D#5", "G5",

    "D#6", "D#6", "D#6", "D#6", "G5",  "G5",  "D#6", "D#6", "D#6", "D#6", "G5",
    "G5",  "D#6", "D#6", "G5",  "G5",  "D#6", "D#6", "D#6", "D#6", "G5",  "G5",
    "D#6", "D#6", "D#6", "D#6", "G5",  "G5",  "D#6", "D#6", "G5",  "G5",

    "G6",  "G6",  "G6",  "G6",  "C6",  "C6",  "G6",  "G6",  "G6",  "G6",  "C6",
    "C6",  "G6",  "G6",  "C6",  "C6",  "G6",  "G6",  "G6",  "G6",  "C6",  "C6",
    "G6",  "G6",  "G6",  "G6",  "C6",  "C6",  "G6",  "G6",  "C6",  "C6",

    "F6",  "F6",  "F6",  "F6",  "G#5", "G#5", "F6",  "F6",  "F6",  "F6",  "G#5",
    "G#5", "F6",  "F6",  "G#5", "G#5", "F6",  "F6",  "F6",  "F6",  "G#5", "G#5",
    "F6",  "F6",  "F6",  "F6",  "G#5", "G#5", "F6",  "F6",  "G#5", "G#5",

    "D#6", "D#6", "D#6", "D#6", "G5",  "G5",  "D#6", "D#6", "D#6", "D#6", "G5",
    "G5",  "D#6", "D#6", "G5",  "G5",  "G5",  "D6",  "D#6", "F6",  "D6",  "D#6",
    "F6",  "D#6",

    "F6",  "F6",  "G6",  "G6",  "G6",  "G6",  "G6",  "D#6", "D#6", "D#6", "D#6",
    "G5",  "G5",  "D#6", "D#6", "D#6", "D#6", "G5",  "G5",  "D#6", "D#6", "G5",
    "G5",  "D#6", "D#6", "D#6", "D#6", "G5",  "G5",  "D#6", "D#6", "D#6", "D#6",
    "G5",  "G5",  "D#6", "D#6", "G5",  "G5",

    "G6",  "G6",  "G6",  "G6",  "C6",  "C6",  "G6",  "G6",  "G6",  "G6",  "C6",
    "C6",  "G6",  "G6",  "C6",  "C6",  "G6",  "G6",  "G6",  "G6",  "C6",  "C6",
    "G6",  "G6",  "G6",  "G6",  "C6",  "C6",  "G6",  "G6",  "C6",  "C6",

    "F6",  "F6",  "F6",  "F6",  "G#5", "G#5", "F6",  "F6",  "F6",  "F6",  "G#5",
    "G#5", "F6",  "F6",  "G#5", "G#5", "F6",  "F6",  "F6",  "F6",  "G#5", "G#5",
    "F6",  "F6",  "F6",  "F6",  "G#5", "G#5", "F6",  "F6",  "G#5", "G#5",

    "D#6", "C6",  "G#5", "D#5", "D#5", "G5",  "G#5", "A#5", "D#6", "C6",  "G#5",
    "D#5", "G5",  "G#5", "A#5", "C6",

    "F6",  "F6",  "F6",  "F6",  "F6",  "F6",  "F6",  "F6",  "F6",  "F6",  "F6",
    "F6",  "F6",  "F6",  "F6",  "F6",

    "G6",  "G6",  "G6",  "G6",  "G6",  "G6",  "F6",  "F6",  "F6",  "F6",  "F6",
    "F6",  "G6",  "G6",  "G6",  "G6",  "G6",  "G6",  "G6",  "G6",  "G6",  "G6",
    "F6",  "F6",  "F6",  "F6",  "F6",  "F6",  "G6",  "G6",  "G6",  "G6",  "G#6",
    "G#6", "G#6", "G#6", "G#6", "G#6", "G6",  "G6",  "G6",  "G6",  "G6",  "G6",
    "G#6", "G#6", "G#6", "G#6", "G#6", "G#6", "G#6", "G#6", "G#6", "G#6", "G6",
    "G6",  "G6",  "G6",  "G6",  "G6",  "G#6", "G#6", "G#6", "G#6", "D6",  "D6",
    "D6",  "D6",  "D6",  "D6",  "A#6", "A#6", "A#6", "A#6", "A#6", "A#6", "D6",
    "D6",  "D6",  "D6",  "D6",  "D6",  "D6",  "D6",  "D6",  "D6",  "A#6", "A#6",
    "A#6", "A#6", "A#6", "A#6", "D6",  "D6",  "D6",  "D6",

    "A#4", "C5",  "D#5", "F5",  "G5",  "F5",  "G5",  "G#5", "A#5", "G#5", "G5",
    "F5",  "D#5", "D5",  "C5",  "A#4", "C5",  "G5",  "C6",  "D#6", "G6",  "G6",
    "G6",  "D#6", "C6",  "C6",  "G5",  "C6",  "D#6", "D#6", "G6",  "G6",  "A#6",
    "C4",  "A#6", "C4",  "A#6", "C4",  "A#6", "C4",  "A#6", "C4",  "A#6", "C4",
    "A#6", "C4",  "A#6", "C4",  "A#6", "C4",  "A#6", "C4",  "A#6", "C4",  "A#6",
    "C4",  "G4",  "G4",  "A#4", "A#4", "C5",  "C5"};

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
  ImGui::PushItemWidth(120.f * uiScale);
  ImGui::InputScalar("Slot", ImGuiDataType_U8, Max::get().slot_number(), NULL,
                     NULL, "%d", ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine(0, 4);
  if (ImGui::Button("Save game")) {
    *Max::get().spawn_room() = *Max::get().player_room();
    Max::get().save_game();
  }
  Tooltip("This sets your current room as spawn and runs the save function "
          "anywhere.\nIn rooms without a phone you will spawn near the "
          "top left corner.");
  if (ImGui::CollapsingHeader("Equipment##PlayerEquipment"))
    Flags(equipment_names, Max::get().equipment(), false);
  if (ImGui::CollapsingHeader("Items##PlayerItems"))
    Flags(item_names, Max::get().items(), false);
  if (ImGui::CollapsingHeader("Miscellaneous##PlayerMisc"))
    Flags(misc_names, Max::get().upgrades(), false);

  if (ImGui::CollapsingHeader("Consumables##PlayerConsumables")) {
    ImGui::DragScalar("Health", ImGuiDataType_S8, Max::get().player_hp(), 0.1f);
    ImGui::DragScalar("More health", ImGuiDataType_S8,
                      Max::get().player_hp() + 1, 0.1f);
    ImGui::DragScalar("Keys", ImGuiDataType_U8, Max::get().keys(), 0.1f);
    ImGui::DragScalar("Matches", ImGuiDataType_U8, Max::get().keys() + 1, 0.1f);
    ImGui::DragScalar("Firecrackers", ImGuiDataType_U8, Max::get().keys() + 2,
                      0.1f);
  }
  if (ImGui::CollapsingHeader("Position##PlayerPosition")) {
    ImGui::InputInt2("Room", &Max::get().player_room()->x);
    ImGui::InputFloat2("Position", &Max::get().player_position()->x);
    ImGui::InputFloat2("Velocity", &Max::get().player_velocity()->x);
    ImGui::InputInt2("Spawn room", &Max::get().spawn_room()->x);
    ImGui::InputInt2("Respawn room", &Max::get().respawn_room()->x);
    ImGui::InputInt2("Respawn tile", &Max::get().respawn_position()->x);
    ImGui::InputInt("Map", Max::get().player_map());
    ImGui::InputFloat2("Wheel", &Max::get().player_wheel()->x);
  }
  if (ImGui::CollapsingHeader("State##PlayerState")) {
    ImGui::InputScalar("State", ImGuiDataType_U8, Max::get().player_state());
    ImGui::InputScalar("Flute", ImGuiDataType_U8, Max::get().player_flute());
    ImGui::InputScalar("Item", ImGuiDataType_U8, Max::get().item());
    ImGui::InputScalar("Ingame time", ImGuiDataType_U32, Max::get().timer());
    ImGui::InputScalar("Total time", ImGuiDataType_U32, Max::get().timer() + 1);
    ImGui::Checkbox("Paused", &Max::get().pause()->paused);
  }
  if (ImGui::CollapsingHeader("Warp##PlayerWarp")) {
    ImGui::InputInt2("Warp room", &Max::get().warp_room()->x);
    ImGui::InputInt2("Warp position", &Max::get().warp_position()->x);
    ImGui::InputInt("Warp map", Max::get().warp_map());
    if (ImGui::Button(
            fmt::format("Warp ({})", ImGui::GetKeyChordName(keys["warp"]))
                .c_str()))
      doWarp = true;
    // TODO: Add saveable custom warp positions from current player/warp
    // position
  }
  ImGui::PopItemWidth();
}

void UI::DrawMap() {
  ImGuiIO &io = ImGui::GetIO();
  ImGuiContext &g = *GImGui;

  ImVec2 realmapsize{800 * uiScale, 528 * uiScale};
  ImVec2 roomsize{40 * uiScale, 22 * uiScale};

  static const std::map<int, std::pair<S32Vec2, S32Vec2>> areas{
      //{0, {{2, 4}, {18, 20}}},
      {1, {{10, 11}, {13, 13}}},
      {2, {{7, 4}, {14, 20}}},
      {3, {{8, 7}, {14, 10}}},
      {4, {{11, 12}, {12, 13}}}};
  ImVec2 bordersize{realmapsize.x / 20 * 2, realmapsize.y / 24 * 4};
  ImVec2 mapsize{realmapsize.x - bordersize.x * 2,
                 realmapsize.y - bordersize.y * 2};
  ImVec2 uv0{bordersize.x / realmapsize.x, bordersize.y / realmapsize.y};
  ImVec2 uv1{1 - uv0.x, 1 - uv0.y};
  if (!options["map_small"].value) {
    mapsize = realmapsize;
    bordersize = {0, 0};
    uv0 = {0, 0};
    uv1 = {1, 1};
  }
  static S32Vec2 cpos{0, 0};
  static S32Vec2 wroom{0, 0};
  static S32Vec2 wpos{0, 0};
  static int layer{0};
  ImGui::PushItemWidth(0.2f * mapsize.x);
  ImGui::InputInt2("Room##MinimapRoom", &wroom.x);
  ImGui::SameLine(0.30f * mapsize.x);
  ImGui::InputInt2("Position##MinimapPosition", &wpos.x);
  ImGui::SameLine(0.62f * mapsize.x);
  ImGui::InputInt("Map##MinimapMap", &layer);
  layer = (layer + 5) % 5;
  ImGui::PopItemWidth();
  ImGui::SameLine(mapsize.x - 60.f * uiScale);
  if (ImGui::Button("Refresh##MinimapRefresh",
                    ImVec2(60.f * uiScale + ImGui::GetStyle().WindowPadding.x,
                           ImGui::GetItemRectSize().y)) ||
      (((options["map_auto"].value || io.MouseDown[1]) &&
        ImGui::GetFrameCount() > lastMinimapFrame + 15)) ||
      ImGui::IsWindowAppearing() || !minimap_init) {
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
    Tooltip("Right click the map to warp\nanywhere on current layer.");
    if (ImGui::IsItemHovered()) {
      cpos.x = ((b.x - d.x) - a.x + c.x + bordersize.x);
      cpos.y = ((b.y - d.y) - a.y + c.y + bordersize.y);
      wroom.x = cpos.x / realmapsize.x * 800 / 40;
      wroom.y = cpos.y / realmapsize.y * 528 / 22;
      wpos.x = ((int)(cpos.x / realmapsize.x * 800) % 40) * 8;
      wpos.y = ((int)(cpos.y / realmapsize.y * 528) % 22) * 8;
      if (io.MouseDown[1]) {
        *Max::get().player_state() = 18;
        *Max::get().warp_room() = wroom;
        *Max::get().warp_position() = wpos;
        *Max::get().warp_map() = layer;
        doWarp = true;
      } else if (io.MouseReleased[1] && *Max::get().player_state() == 18) {
        *Max::get().player_state() = 0;
      }
      {
        auto ax = wroom.x * roomsize.x;
        auto ay = wroom.y * roomsize.y;
        auto bx = ax + roomsize.x;
        auto by = ay + roomsize.y;
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(a.x + d.x + ax - c.x - bordersize.x,
                   a.y + d.y + ay - c.y - bordersize.y),
            ImVec2(a.x + d.x + bx - c.x - bordersize.x,
                   a.y + d.y + by - c.y - bordersize.y),
            0xccffffff, 0, 0, 1.0f);
      }
    }
    ImGui::PopStyleColor(3);

    if (options["map_areas"].value) {
      for (auto &[l, box] : areas) {
        auto ax = box.first.x * roomsize.x;
        auto ay = box.first.y * roomsize.y;
        auto bx = box.second.x * roomsize.x;
        auto by = box.second.y * roomsize.y;
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(a.x + d.x + ax - c.x - bordersize.x,
                   a.y + d.y + ay - c.y - bordersize.y),
            ImVec2(a.x + d.x + bx - c.x - bordersize.x,
                   a.y + d.y + by - c.y - bordersize.y),
            0xff00eeee, 0, 0, 1.0f);
      }
    }

    if (areas.contains(layer)) {
      auto ax = areas.at(layer).first.x * roomsize.x;
      auto ay = areas.at(layer).first.y * roomsize.y;
      auto bx = areas.at(layer).second.x * roomsize.x;
      auto by = areas.at(layer).second.y * roomsize.y;
      ImGui::GetWindowDrawList()->AddRect(
          ImVec2(a.x + d.x + ax - c.x - bordersize.x,
                 a.y + d.y + ay - c.y - bordersize.y),
          ImVec2(a.x + d.x + bx - c.x - bordersize.x,
                 a.y + d.y + by - c.y - bordersize.y),
          0xff00ff00, 0, 0, 3.0f);
    }

    auto pl = *Max::get().player_map();
    if (areas.contains(pl)) {
      auto ax = areas.at(pl).first.x * roomsize.x;
      auto ay = areas.at(pl).first.y * roomsize.y;
      auto bx = areas.at(pl).second.x * roomsize.x;
      auto by = areas.at(pl).second.y * roomsize.y;
      ImGui::GetWindowDrawList()->AddRect(
          ImVec2(a.x + d.x + ax - c.x - bordersize.x,
                 a.y + d.y + ay - c.y - bordersize.y),
          ImVec2(a.x + d.x + bx - c.x - bordersize.x,
                 a.y + d.y + by - c.y - bordersize.y),
          0xff0000ff, 0, 0, 3.0f);
    }

    {
      auto px = Max::get().warp_room()->x * roomsize.x +
                (Max::get().warp_position()->x / 320.f * roomsize.x);
      auto py = Max::get().warp_room()->y * roomsize.y +
                (Max::get().warp_position()->y / 180.f * roomsize.y);
      ImGui::GetWindowDrawList()->AddCircleFilled(
          ImVec2(a.x + d.x + px - c.x - bordersize.x,
                 a.y + d.y + py - c.y - bordersize.y),
          3.f, 0xff00eeee);
    }

    {
      auto px = Max::get().player_room()->x * roomsize.x +
                (Max::get().player_position()->x / 320.f * roomsize.x);
      auto py = Max::get().player_room()->y * roomsize.y +
                (Max::get().player_position()->y / 180.f * roomsize.y);
      ImGui::GetWindowDrawList()->AddCircleFilled(
          ImVec2(a.x + d.x + px - c.x - bordersize.x,
                 a.y + d.y + py - c.y - bordersize.y),
          3.f, 0xee0000ee);
    }

    if (options["map_wheel"].value) {
      auto px = Max::get().player_room()->x * roomsize.x +
                (Max::get().player_wheel()->x / 320.f * roomsize.x);
      auto py = Max::get().player_room()->y * roomsize.y +
                (Max::get().player_wheel()->y / 180.f * roomsize.y);
      while (px < 80.f * uiScale)
        px += 640.f * uiScale;
      while (px > 720.f * uiScale)
        px -= 640.f * uiScale;
      while (py > 440.f * uiScale)
        py -= 352.f * uiScale;
      ImGui::GetWindowDrawList()->AddCircle(
          ImVec2(a.x + d.x + px - c.x - bordersize.x,
                 a.y + d.y + py - c.y - bordersize.y),
          4.f, 0xee00ffee, 0, 1.5f);
    }
  }
}

void UI::ScaleWindow() {
  if (*Max::get().options() & 2) // FS
    return;
  RECT c;
  RECT w;
  GetClientRect(hWnd, &c);
  GetWindowRect(hWnd, &w);
  int dx = (w.right - w.left) - (c.right - c.left);
  int dy = (w.bottom - w.top) - (c.bottom - c.top);
  SetWindowPos(hWnd, NULL, 0, 0, windowScale * 320 + dx, windowScale * 180 + dy,
               2);
}

void UI::DrawOptions() {
  ImGuiIO &io = ImGui::GetIO();
  ImGui::PushItemWidth(120.f * uiScale);
  bool noclip = options["cheat_noclip"].value;
  for (auto &[name, enabled] : options) {
    Option(name);
  }
  if (noclip && !options["cheat_noclip"].value)
    *Max::get().player_state() = 0;
  if (ImGui::SliderInt("Window scale", &windowScale, 1, 10, "%dx")) {
    ScaleWindow();
  }
  ImGui::SliderFloat("Alpha", &ImGui::GetStyle().Alpha, 0.2f, 1.0f, "%.1f");
  if (Button("Save settings"))
    SaveINI();
  if (Button("Load settings"))
    LoadINI();
  ImGui::PopItemWidth();
}

bool UI::Option(std::string name) {
  std::string title =
      options[name].name +
      (options[name].key != ""
           ? " (" +
                 std::string(ImGui::GetKeyChordName(keys[options[name].key])) +
                 ")"
           : "");
  bool ret = false;
  if (inMenu)
    ret = ImGui::MenuItem(title.c_str(), "", &options[name].value);
  else
    ret = ImGui::Checkbox(title.c_str(), &options[name].value);
  Tooltip(options[name].desc);
  return ret;
}

bool UI::Button(std::string name, std::string desc, std::string key) {
  std::string title =
      name + (key != ""
                  ? " (" + std::string(ImGui::GetKeyChordName(keys[key])) + ")"
                  : "");
  bool ret = false;
  if (inMenu)
    ret = ImGui::MenuItem(title.c_str());
  else
    ret = ImGui::Button(title.c_str());
  if (desc != "")
    Tooltip(desc);
  return ret;
}

std::string Timestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto in_time_t = system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  return ss.str();
}

std::string TimestampFile() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto in_time_t = system_clock::to_time_t(now);
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
  ss << '_' << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

void UI::ScreenShot() {
  screenShotNextFrame = screenShotFileName + "_" + TimestampFile();
}

uint8_t *GetMural() {
  static uint8_t mural[20 * 40]{0};
  auto m = Max::get().mural();
  int i = 0;
  do {
    mural[4 * i] = (*m)[i] & 0x3;
    mural[4 * i + 1] = ((*m)[i] & 0xc) >> 2;
    mural[4 * i + 2] = ((*m)[i] & 0x30) >> 4;
    mural[4 * i + 3] = ((*m)[i] & 0xc0) >> 6;
    i++;
  } while (i < 200);
  return mural;
}

void UI::LoadMuralPage(int page) {
  sequencer.pages[sequencer.page_loaded] = *Max::get().mural();
  sequencer.page_loaded = page;
  sequencer.page = page;
  *Max::get().mural() = sequencer.pages[sequencer.page_loaded];
}

void UI::DrawTools() {
  ImGuiIO &io = ImGui::GetIO();
  ImGui::PushItemWidth(120.f * uiScale);
  if (ImGui::CollapsingHeader("Screen shooter  ")) {
    ImGui::InputText("File prefix", &screenShotFileName);
    ImGui::InputInt2("Room range", &screenShotRange.x);
    if (ImGui::Button("Capture (.)"))
      ScreenShot();
    ImGui::SameLine(0, 4);
    if (ImGui::Button("Capture range")) {
      *Max::get().warp_room() = *Max::get().player_room();
      Max::get().warp_position()->x = (int)Max::get().player_position()->x;
      Max::get().warp_position()->y = (int)Max::get().player_position()->y;
      *Max::get().warp_map() = *Max::get().player_map();
      screenShotIndex = 0;
      screenShotFrame = 0;
    }
  }
  if (ImGui::CollapsingHeader("Bunny sequencer  ")) {
    /*if (ImGui::Button("TTFAF")) {
      *Max::get().item() = 2;
      *Max::get().player_state() = 7;
      for (auto note : ttfaf) {
        std::string s(note);
        if (!notes.contains(s))
          continue;
        for (int i = 0; i < 4; ++i)
          Max::get().inputs.push_back(notes.at(s) | 0x4000);
        Max::get().inputs.push_back(0);
      }
      Max::get().inputs.push_back(-1);
    }*/
    if (ImGui::Checkbox("Enable sequencer", &sequencer.enabled)) {
      Max::get().input = PLAYER_INPUT::SKIP;
      Max::get().inputs.clear();
    }
    if (ImGui::Button("Clear queue")) {
      Max::get().input = PLAYER_INPUT::SKIP;
      Max::get().inputs.clear();
    }
    ImGui::InputInt("Base##NoteBase", &sequencer.base);
    if (sequencer.base < 0)
      sequencer.base = 0;
    if (sequencer.base > 5)
      sequencer.base = 5;
    ImGui::InputInt("Duration##NoteDuration", &sequencer.duration);
    if (sequencer.duration < 2)
      sequencer.duration = 2;
    if (sequencer.duration > 40)
      sequencer.duration = 40;
    ImGui::InputInt("Length##SongLength", &sequencer.length);
    if (sequencer.length < 1)
      sequencer.length = 1;
    if (sequencer.length > 40)
      sequencer.length = 40;
    ImGui::InputInt("Page count##SongCount", &sequencer.page_count);
    if (sequencer.page_count < 1)
      sequencer.page_count = 1;
    ImGui::InputInt("Page##SongPage", &sequencer.page);
    if (sequencer.page < 1)
      sequencer.page = 1;
    if (sequencer.page > sequencer.page_count)
      sequencer.page_count = sequencer.page;
    if (sequencer.page_loaded != sequencer.page) {
      LoadMuralPage(sequencer.page);
    }
    ImGui::LabelText(
        "Note", "%s",
        note_order.at(19 - Max::get().mural_selection()[1] + sequencer.base));
    ImGui::LabelText("X", "%d", Max::get().mural_selection()[0]);
    ImGui::LabelText("Y", "%d", Max::get().mural_selection()[1]);
    ImGui::LabelText("Queue", "%d", Max::get().inputs.size());
  }
  ImGui::PopItemWidth();
}

void UI::Play() {
  if (sequencer.enabled && Max::get().player_room()->x == 13 &&
      Max::get().player_room()->y == 11) {
    if (get_address("mural_cursor")) {
      write_mem_recoverable("mural_cursor", get_address("mural_cursor"),
                            get_nop(2), true);
    }
    if (*Max::get().player_state() == 7 &&
        (*Max::get().timer() % sequencer.duration) == 0) {
      sequencer.note.clear();
      sequencer.a = std::nullopt;
      sequencer.b = std::nullopt;
      for (int i = 0; i < sequencer.duration; ++i)
        sequencer.note[i] = 0;
      Max::get().mural_selection()[0] =
          (Max::get().mural_selection()[0] + 1) % sequencer.length;
      if (Max::get().mural_selection()[0] == 0 &&
          sequencer.page < sequencer.page_count) {
        LoadMuralPage(sequencer.page + 1);
      } else if (Max::get().mural_selection()[0] == 0 &&
                 sequencer.page == sequencer.page_count) {
        LoadMuralPage(1);
      }
      auto *m = GetMural();
      for (int dy = 19; dy >= 0; dy--) {
        int p = m[dy * 40 + Max::get().mural_selection()[0]];
        int a = 0;
        int b = 0;
        if (p == 1) {
          a = 0;
          b = sequencer.duration / 2;
          sequencer.a = dy;
        } else if (p == 2) {
          a = sequencer.duration / 2;
          b = sequencer.duration;
          sequencer.b = dy;
        } else if (p == 3) {
          a = 0;
          b = sequencer.duration;
          sequencer.a = dy;
          sequencer.b = dy;
        }
        for (int i = a; i < b; ++i)
          sequencer.note[i] =
              notes.at(note_order.at(19 - dy + sequencer.base)) | 0x4000;
        if (p > 0)
          Max::get().mural_selection()[1] = dy;
        if (p == 3)
          continue;
      }
      for (auto &[f, n] : sequencer.note) {
        Max::get().inputs.push_back(n);
      }
    } else if (sequencer.enabled && (*Max::get().player_state() == 0 ||
                                     Max::get().pause()->paused)) {
      Max::get().input = PLAYER_INPUT::SKIP;
      Max::get().inputs.clear();
    }
    if (Max::get().inputs.size() > sequencer.duration / 2 &&
        sequencer.a.has_value())
      Max::get().mural_selection()[1] = sequencer.a.value();
    else if (Max::get().inputs.size() > 0 && sequencer.b.has_value())
      Max::get().mural_selection()[1] = sequencer.b.value();
    if (!Max::get().pause()->paused)
      Max::get().render_queue.push_back(
          [&]() { Max::get().draw_text(57, 12, L"bunny sequencer 0.2"); });
  }
  if (!sequencer.enabled)
    recover_mem("mural_cursor");
}

void UI::RefreshMaps() {
  maps.clear();
  CreateDirectory(L"MAXWELL\\Maps", NULL);
  if (std::filesystem::exists(mapDir) &&
      std::filesystem::is_directory(mapDir)) {
    for (const auto &file : std::filesystem::directory_iterator(mapDir)) {
      maps.push_back(file.path());
    }
  }
}

void UI::DrawTile(Tile &tile) {
  ImGui::InputScalar("ID", ImGuiDataType_U16, &tile.id, &u16_one);
  ImGui::InputScalar("Param", ImGuiDataType_U8, &tile.param, &u8_one);
  ImGui::InputScalar("Flags", ImGuiDataType_U8, &tile.flags, &u8_one);
}

void UI::DrawTileRow(Tile &tile) {
  ImGui::PushItemWidth(40.f * uiScale);
  ImGui::InputScalar("##ID", ImGuiDataType_U16, &tile.id, nullptr, nullptr);
  ImGui::SameLine(0, 4);
  ImGui::InputScalar("##Param", ImGuiDataType_U8, &tile.param, nullptr,
                     nullptr);
  ImGui::SameLine(0, 4);
  ImGui::InputScalar("##Flags", ImGuiDataType_U8, &tile.flags, nullptr,
                     nullptr);
  ImGui::PopItemWidth();
}

void UI::DrawSelectedTile(SelectedTile &tile) {
  ImGui::InputInt2("Room", &tile.room.x, ImGuiInputTextFlags_ReadOnly);
  ImGui::InputInt2("Position", &tile.pos.x, ImGuiInputTextFlags_ReadOnly);
  ImGui::InputInt("Layer", &tile.layer, 0, 0, ImGuiInputTextFlags_ReadOnly);
  ImGui::InputInt("Map", &tile.map, 0, 0, ImGuiInputTextFlags_ReadOnly);
  DrawTile(*tile.tile);
}

void UI::DrawSelectedTileRow(SelectedTile &tile) {
  DrawTileRow(*tile.tile);
  ImGui::SameLine(0, 4);
  ImGui::Text("%d,%d %d,%d %s", tile.room.x, tile.room.y, tile.pos.x,
              tile.pos.y, (tile.layer ? "BG" : "FG"));
  ImGui::SameLine(ImGui::GetContentRegionMax().x - 24.f * uiScale, 0);
  if (ImGui::Button("Go", ImVec2(24.f * uiScale, ImGui::GetFrameHeight()))) {
    *Max::get().warp_map() = tile.map;
    *Max::get().warp_room() = tile.room;
    Max::get().warp_position()->x = 8 * tile.pos.x;
    Max::get().warp_position()->y = 8 * tile.pos.y;
    doWarp = true;
  }
}

void ColorEdit3(const char *label, uint8_t *col, ImGuiColorEditFlags flags = 0) {
    float col4[4] { col[0] / 255.0f, col[1] / 255.0f, col[2] / 255.0f, 1.0f };

    ImGui::ColorEdit4(label, col4, flags | ImGuiColorEditFlags_NoAlpha);

    col[0] = col4[0] * 255.f;
    col[1] = col4[1] * 255.f;
    col[2] = col4[2] * 255.f;
}

void ColorEdit4(const char *label, uint8_t *col, ImGuiColorEditFlags flags = 0) {
    float col4[4] { col[0] / 255.0f, col[1] / 255.0f, col[2] / 255.0f, col[3] / 255.0f };

    ImGui::ColorEdit4(label, col4, flags);

    col[0] = col4[0] * 255.f;
    col[1] = col4[1] * 255.f;
    col[2] = col4[2] * 255.f;
    col[3] = col4[3] * 255.f;
}

void UI::DrawLevel() {
  ImGuiIO &io = ImGui::GetIO();
  ImGuiContext &g = *GImGui;

  if (ImGui::CollapsingHeader("Room")) {
    static bool lockCurrentRoom{true};
    ImGui::Checkbox("Select current room", &lockCurrentRoom);
    if (lockCurrentRoom) {
      selectedRoom.pos = *Max::get().player_room();
      selectedRoom.map = *Max::get().player_map();
      selectedRoom.room = Max::get().room(selectedRoom.map, selectedRoom.pos.x,
                                          selectedRoom.pos.y);
    }

    ImGui::BeginDisabled(lockCurrentRoom);
    if(ImGui::InputInt2("Position##RoomPosition", &selectedRoom.pos.x)) {
      selectedRoom.room = Max::get().room(selectedRoom.map, selectedRoom.pos.x, selectedRoom.pos.y);
    }
    ImGui::EndDisabled();

    if (selectedRoom.room) {
      ImGui::InputScalarN("BG##RoomBG", ImGuiDataType_U8,
                          &selectedRoom.room->bgId, 1, &u8_one);
      ImGui::InputScalarN("Pallet Index##RoomPalette", ImGuiDataType_U8,
                          &selectedRoom.room->params.palette, 1, &u8_one);
      ImGui::InputScalarN("???##UnknownRoomParams", ImGuiDataType_U8,
                          &selectedRoom.room->params.idk1, 3);
      ImGui::DragScalar("Water level##RoomWaterLevel", ImGuiDataType_U8,
                        &selectedRoom.room->waterLevel, 0.1f, &u8_min, &u8_max);
      ImGui::InputScalar("##ForcedPalette", ImGuiDataType_U8, &forcedPalette,
                         &u8_one);
      if (forcedPalette > 31)
        forcedPalette = 31;
      ImGui::SameLine(0, 4);
      ImGui::Checkbox("Forced palette", &options["cheat_palette"].value);
      if (options["cheat_palette"].value) {
        Max::get().force_palette = forcedPalette;
      } else {
        Max::get().force_palette = std::nullopt;
      }

      ImGui::SeparatorText("Pallet Data");

      auto amb = Max::get().lighting(selectedRoom.room->params.palette);

      if(amb) {
        if(ImGui::Button("Reset")) {
            Max::get().resetLighting(selectedRoom.room->params.palette);
        }
        ColorEdit3("ambient light", amb->ambient_light);
        ColorEdit3("fg ambient multi", amb->fg_ambient_multi);
        ColorEdit3("bg ambient multi", amb->bg_ambient_multi);

        ColorEdit4("lamp intensity", amb->light_intensity);
        ImGui::DragFloat3("color dividers", amb->dividers);
        ImGui::DragFloat("color saturation", &amb->saturation);
        ImGui::DragFloat("bg texture light multi", &amb->bg_tex_light_multi);
      }
    }
  }

  ImGui::PushID("TileEditor");
  if (ImGui::CollapsingHeader("Tile editor   ")) {
    ImGui::SeparatorText("Tile to place");
    ImGui::PushID("EditorTile");
    DrawTileRow(editorTile);
    ImGui::PopID();
    if (selectedTile.tile) {
      ImGui::SeparatorText("Selected tile");
      ImGui::PushID("SelectedTile");
      DrawSelectedTileRow(selectedTile);
      ImGui::PopID();
    }
  }
  ImGui::PopID();

  ImGui::PushID("TileSearch");
  if (ImGui::CollapsingHeader("Tile search   ")) {
    static uint16_t searchId{0};
    ImGui::InputScalar("ID", ImGuiDataType_U16, &searchId, &u16_one);
    const bool focused = ImGui::IsItemFocused();
    bool doSearch = false;
    bool doClear = false;
    ImGui::SameLine(130.f * uiScale, 4);
    if (ImGui::Button("Search##SearchTiles") ||
        (focused && ImGui::IsKeyPressed(ImGuiKey_Enter))) {
      doSearch = true;
      doClear = !ImGui::IsKeyDown(ImGuiMod_Ctrl);
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button("Add##AddTiles")) {
      doSearch = true;
      doClear = false;
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button("Clear##ClearTiles"))
      doClear = true;
    if (doClear)
      searchTiles.clear();
    if (doSearch) {
      auto *map = Max::get().map(*Max::get().player_map());
      for (int r = 0; r < map->roomCount; ++r) {
        for (int l = 0; l < 2; ++l) {
          for (int y = 0; y < 22; ++y) {
            for (int x = 0; x < 40; ++x) {
              Room &room = map->rooms[r];
              if (room.tiles[l][y][x].id == searchId) {
                searchTiles.emplace_back(
                    SelectedTile{&room.tiles[l][y][x],
                                 {room.x, room.y},
                                 {x, y},
                                 l,
                                 *Max::get().player_map()});
              }
            }
          }
        }
      }
    }
    int i = 0;
    ImGui::PushID("TileSearchResults");
    if (searchTiles.size() > 0) {
      ImGui::PushItemWidth(126.f * uiScale);
      std::string label = fmt::format("Found {} tiles:", searchTiles.size());
      ImGui::LabelText("", label.c_str());
      ImGui::SameLine(0, 4);
      std::string buf =
          fmt::format("Set all to {}##SetAllTiles", editorTile.id);
      if (ImGui::Button(buf.c_str())) {
        for (auto &tile : searchTiles) {
          tile.tile->id = editorTile.id;
          tile.tile->param = editorTile.param;
          tile.tile->flags = editorTile.flags;
        }
      }
      ImGui::PopItemWidth();
    } else
      ImGui::TextWrapped(
          "Type tile ID to search and highlight tiles in the current map.");
    for (auto tile : searchTiles) {
      ImGui::PushID(++i);
      DrawSelectedTileRow(tile);
      ImGui::PopID();
    }
    ImGui::PopID();
  }
  ImGui::PopID();

  if (ImGui::CollapsingHeader("Import map")) {
    static bool mapsInit{false};
    if (!mapsInit) {
      RefreshMaps();
      mapsInit = true;
    }
    if (ImGui::Button("Open Maps folder##OpenMaps")) {
      RefreshMaps();
      ShellExecute(NULL, L"open", L"MAXWELL\\Maps", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button("Refresh##RefreshMaps"))
      RefreshMaps();
    static int layer{0};
    ImGui::InputInt("Map##ImportMapLayer", &layer);
    layer = (layer + 5) % 5;
    /*static std::string file{""};
    ImGui::InputText("File name##ImportMapFile", &file);
    if (ImGui::Button("Import##ImportMap"))
      Max::get().import_map(file, layer);*/

    for (auto m : maps) {
      ImGui::PushID(m.string().c_str());
      std::string buf =
          fmt::format("Import '{}' to map {}", m.filename().string(), layer);
      if (ImGui::Button(buf.c_str()))
        Max::get().import_map(m.string(), layer);
      ImGui::PopID();
    }
    if (maps.empty())
      ImGui::TextWrapped(
          "Put files exported by map editor in MAXWELL/Maps to import!");
  }
}

UI::UI(float scale) {
  Max::get();
  LoadINI();
  options["ui_visible"].value = true;

  dpiScale = scale;
  if (options["ui_scaling"].value)
    uiScale = dpiScale;

  NewWindow("F1 Player", keys["tool_player"], 0,
            [this]() { this->DrawPlayer(); });
  NewWindow("F2 Minimap", keys["tool_map"], ImGuiWindowFlags_AlwaysAutoResize,
            [this]() { this->DrawMap(); });
  NewWindow("F3 Tools", keys["tool_tools"], 0, [this]() { this->DrawTools(); });
  NewWindow("F4 Level", keys["tool_level"], 0, [this]() { this->DrawLevel(); });
  NewWindow("Settings", keys["tool_settings"],
            ImGuiWindowFlags_AlwaysAutoResize,
            [this]() { this->DrawOptions(); });
  NewWindow("Debug", ImGuiKey_None, 0, [this]() {
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SeparatorText("Patterns");
    for (auto &[name, addr] : get_addresses()) {
      if (!addr)
        ImGui::PushStyleColor(ImGuiCol_Text, 0xff0000ff);
      ImGui::InputScalar(name.data(), ImGuiDataType_U64, &addr, NULL, NULL,
                         "%p", ImGuiInputTextFlags_ReadOnly);
      if (!addr)
        ImGui::PopStyleColor();
    }

    size_t v = (size_t)Max::get().slot();
    ImGui::InputScalar("save slot", ImGuiDataType_U64, &v, NULL, NULL, "%p",
                       ImGuiInputTextFlags_ReadOnly);

    v = (size_t)Max::get().pause();
    ImGui::InputScalar("pause", ImGuiDataType_U64, &v, NULL, NULL, "%p",
                       ImGuiInputTextFlags_ReadOnly);

    v = (size_t)Max::get().map(0);
    ImGui::InputScalar("world", ImGuiDataType_U64, &v, NULL, NULL, "%p",
                       ImGuiInputTextFlags_ReadOnly);

    if (ImGui::Button("Reload map mods"))
      Max::get().load_map_mods();

    {
      uint8_t *m = GetMural();
      std::string str;
      for (int y = 0; y < 20; ++y) {
        for (int x = 0; x < 40; ++x) {
          int p = m[y * 40 + x];
          if (p == 0)
            str += " ";
          else
            str += "x";
        }
        str += "\n";
      }
      ImGui::Text("%s", str.c_str());
    }

    /*if (!this->inMenu) {
      ImGui::ShowDemoWindow();
      ImGui::ShowMetricsWindow();
      if (ImGui::Begin("Styles")) {
        ImGui::ShowStyleEditor();
        ImGui::End();
      }
    }*/
  });
}

UI::~UI() { Max::get().unhook(); }

bool UI::Keys() {
  if (ImGui::IsKeyReleased((ImGuiKey)keys["escape"]))
    ImGui::SetWindowFocus(nullptr);
  else if (ImGui::IsKeyChordPressed(keys["toggle_ui"]))
    options["ui_visible"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_mouse"]))
    options["input_mouse"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_noclip"])) {
    options["cheat_noclip"].value ^= true;
    if (!options["cheat_noclip"].value)
      *Max::get().player_state() = 0;
  } else if (ImGui::IsKeyChordPressed(keys["toggle_godmode"]))
    options["cheat_godmode"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_damage"]))
    options["cheat_damage"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_darkness"]))
    options["cheat_darkness"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_lights"]))
    options["cheat_lights"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_palette"])) {
    options["cheat_palette"].value ^= true;
    if (options["cheat_palette"].value) {
      Max::get().force_palette = forcedPalette;
    } else {
      Max::get().force_palette = std::nullopt;
    }
  } else if (ImGui::IsKeyChordPressed(keys["toggle_gameboy"]))
    options["cheat_gameboy"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_hud"]))
    options["cheat_hud"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_player"]))
    options["cheat_player"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["warp"]))
    doWarp = true;
  else if (ImGui::IsKeyChordPressed(keys["screenshot"]) &&
           !ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow))
    ScreenShot();
  else if (ImGui::IsKeyChordPressed(keys["pause"])) {
    paused ^= true;
    Max::get().set_pause = paused;
  } else if (ImGui::IsKeyChordPressed(keys["skip"], ImGuiInputFlags_Repeat) &&
             !ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow))
    Max::get().skip = true;
  else
    return false;
  return true;
}

ImVec2 Mouse() {
  ImVec2 base = ImGui::GetMainViewport()->Pos;
  return ImVec2(ImGui::GetIO().MousePos.x - base.x,
                ImGui::GetIO().MousePos.y - base.y);
}

ImVec2 Base() {
  ImVec2 base = ImGui::GetMainViewport()->Pos;
  return ImVec2(base.x, base.y);
}

ImVec2 TileToScreen(ImVec2 tile) {
  ImVec2 base = ImGui::GetMainViewport()->Pos;
  ImVec2 size = ImGui::GetMainViewport()->Size;
  return ImVec2(tile.x * size.x / 40.f + base.x,
                tile.y * size.y / 22.5f + base.y);
}

void UI::Cheats() {
  if (doWarp && get_address("warp")) {
    Max::get().decrypt_stuff();
    write_mem_recoverable("warp", get_address("warp"), "EB"_gh, true);
  } else {
    recover_mem("warp");
  }

  if (options["input_block"].value && get_address("keyboard")) {
    if (Block()) {
      write_mem_recoverable("block", get_address("keyboard"), get_nop(6), true);
    } else {
      recover_mem("block");
    }
  } else {
    recover_mem("block");
  }

  if (options["cheat_damage"].value && get_address("damage")) {
    write_mem_recoverable("damage", get_address("damage"), get_nop(6), true);
  } else {
    recover_mem("damage");
  }

  if (options["cheat_godmode"].value && get_address("god")) {
    write_mem_recoverable("god", get_address("god"), "E9 71 01 00 00 90"_gh,
                          true);
  } else {
    recover_mem("god");
  }

  if (options["cheat_darkness"].value && get_address("render_darkness")) {
    write_mem_recoverable("render_darkness", get_address("render_darkness"),
                          "EB 19"_gh, true);
  } else {
    recover_mem("render_darkness");
  }

  if (options["cheat_lights"].value && get_address("render_lights")) {
    write_mem_recoverable("render_lights", get_address("render_lights"),
                          "E9 8A 00 00 00 90"_gh, true);
  } else {
    recover_mem("render_lights");
  }

  if (options["cheat_gameboy"].value && get_address("render_gameboy")) {
    write_mem_recoverable("render_gameboy", get_address("render_gameboy"),
                          "EB 0E"_gh, true);
  } else {
    recover_mem("render_gameboy");
  }

  if (options["cheat_hud"].value && get_address("render_hud")) {
    write_mem_recoverable(
        "render_hud", get_address("render_hud"), "EB 74"_gh,
        true); // jmp over this and the next if that renders some more text
  } else {
    recover_mem("render_hud");
  }

  if (options["cheat_player"].value && get_address("render_player")) {
    write_mem_recoverable("render_player", get_address("render_player"),
                          "C3"_gh, true);
  } else {
    recover_mem("render_player");
  }

  if (options["cheat_noclip"].value) {
    *Max::get().player_state() = 18;
  }

  if (options["cheat_groundhog"].value && get_address("groundhog_day")) {
    write_mem_recoverable("groundhog_day", get_address("groundhog_day"),
                          get_nop(2), true);
    write_mem_recoverable("groundhog_day2", get_address("groundhog_day") + 26,
                          get_nop(2), true);
  } else {
    recover_mem("groundhog_day");
    recover_mem("groundhog_day2");
  }
}

void UI::Windows() {
  ImGuiIO &io = ImGui::GetIO();
  ImGuiContext &g = *GImGui;
  if (options["ui_visible"].value) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    if (ImGui::BeginMainMenuBar()) {
      ImGui::PopStyleVar(2);
      for (auto *window : windows) {
        if (window->detached)
          continue;
        inMenu = true;
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        if (ImGui::BeginMenu(window->title.c_str(), window->key)) {
          window->cb();
          lastMenuFrame = ImGui::GetFrameCount();
          ImGui::EndMenu();
        }
        Tooltip("Right click to detach a\nwindow from the menu bar.");
        inMenu = false;
        if (io.MouseClicked[1] && ImGui::IsItemHovered())
          window->detached = true;
      }
      ImGui::EndMainMenuBar();
    } else {
      ImGui::PopStyleVar(2);
    }
    for (auto *window : windows) {
      if (!window->detached)
        continue;
      if (ImGui::Begin(window->title.c_str(), &window->detached,
                       window->flags)) {
        window->cb();
      }
      ImGui::End();
    }
  }
}

void UI::HUD() {
  ImGuiIO &io = ImGui::GetIO();
  ImGuiContext &g = *GImGui;
  io.MouseDrawCursor = options["ui_visible"].value;

  {
    using namespace std::chrono_literals;
    auto now = std::chrono::system_clock::now();
    if (io.MousePos.x != lastMousePos.x || io.MousePos.y != lastMousePos.y) {
      lastMouseActivity = now;
      lastMousePos = io.MousePos;
      io.MouseDrawCursor = true;
    } else if (lastMouseActivity + 2s < now) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    }
  }

  {
    std::string version =
        fmt::format("MAXWELL {}", get_version()) + " | GAME " + game_version();
    ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
        ->AddText(ImVec2(io.DisplaySize.x / 2.f -
                             ImGui::CalcTextSize(version.c_str()).x / 2.f +
                             Base().x,
                         io.DisplaySize.y -
                             ImGui::GetTextLineHeightWithSpacing() + Base().y),
                  0x99999999, version.c_str());
  }

  if (options["ui_visible"].value && windowScale > 2) {
    std::string hud =
        fmt::format("{}{}{}{}{}{} ROOM:{},{} POS:{:.0f},{:.0f} {}",
                    options["cheat_damage"].value ? " DAMAGE" : "",
                    options["cheat_noclip"].value ? " NOCLIP" : "",
                    options["cheat_godmode"].value ? " GOD" : "",
                    options["cheat_darkness"].value ? " DARKNESS" : "",
                    options["cheat_lights"].value ? " LIGHTS" : "",
                    options["cheat_palette"].value ? " PALETTE" : "",
                    Max::get().player_room()->x, Max::get().player_room()->y,
                    Max::get().player_position()->x,
                    Max::get().player_position()->y, Timestamp());
    ImGui::GetForegroundDrawList(ImGui::GetMainViewport())
        ->AddText(ImVec2(io.DisplaySize.x - ImGui::CalcTextSize(hud.c_str()).x -
                             ImGui::GetStyle().WindowPadding.x + Base().x,
                         Base().y),
                  0xffffffff, hud.c_str());
  }

  if (options["ui_grid"].value) {
    for (int x = 1; x < 40; ++x) {
      ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
          ->AddLine(TileToScreen(ImVec2(x, 0)), TileToScreen(ImVec2(x, 23)),
                    0x66ffffff, 1.0f);
    }
    for (int y = 1; y < 23; ++y) {
      ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
          ->AddLine(TileToScreen(ImVec2(0, y)), TileToScreen(ImVec2(40, y)),
                    0x66ffffff, 1.0f);
    }
  }

  for (auto &tile : searchTiles) {
    if (tile.map != *Max::get().player_map() ||
        tile.room.x != Max::get().player_room()->x ||
        tile.room.y != Max::get().player_room()->y)
      continue;
    ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
        ->AddRect(TileToScreen(ImVec2(tile.pos.x, tile.pos.y)),
                  TileToScreen(ImVec2(tile.pos.x + 1, tile.pos.y + 1)),
                  (tile.layer ? 0xccffff00 : 0xcc0000ff), 0, 0, 3.f);
  }

  if (ImGui::IsMousePosValid()) {
    auto npos = Normalize(Mouse());
    int x = npos.x;
    int y = npos.y;
    int rx = x / 8;
    int ry = y / 8;

    bool inbound = x > 0 && x < 320 && y > 0 && y < 180;

    if (options["input_mouse"].value && io.MouseDown[1] &&
        !io.WantCaptureMouse) {
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

    auto *fg =
        Max::get().tile(*Max::get().player_map(), Max::get().player_room()->x,
                        Max::get().player_room()->y, rx, ry, 0);
    auto *bg =
        Max::get().tile(*Max::get().player_map(), Max::get().player_room()->x,
                        Max::get().player_room()->y, rx, ry, 1);

    if (fg && options["input_mouse"].value && io.MouseDown[2] &&
        !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
      selectedTile.tile = fg;
      selectedTile.pos = S32Vec2{rx, ry};
      selectedTile.room = *Max::get().player_room();
      selectedTile.map = *Max::get().player_map();
      if (ImGui::IsKeyDown((ImGuiKey)keys["editor_modifier"])) {
        editorTile.id = bg->id;
        editorTile.param = bg->param;
        editorTile.flags = bg->flags;
      } else {
        editorTile.id = fg->id;
        editorTile.param = fg->param;
        editorTile.flags = fg->flags;
      }
    }

    if (fg && options["input_mouse"].value && io.MouseDown[0] &&
        !io.WantCaptureMouse) {
      if (ImGui::IsKeyDown((ImGuiKey)keys["editor_modifier"])) {
        bg->id = editorTile.id;
        bg->param = editorTile.param;
        bg->flags = editorTile.flags;
      } else {
        fg->id = editorTile.id;
        fg->param = editorTile.param;
        fg->flags = editorTile.flags;
      }
    }

    if (options["input_mouse"].value && io.MouseDown[3] &&
        !io.WantCaptureMouse) {
      auto broken = !ImGui::IsKeyDown((ImGuiKey)keys["editor_modifier"]);
      int bit = Max::get().player_room()->y * 20 * 40 * 22 + ry * 20 * 40 +
                Max::get().player_room()->x * 40 + rx;
      Max::get().map_bits(2)->set(bit, broken);
    }

    if (options["ui_coords"].value && inbound &&
        ImGui::GetMouseCursor() != ImGuiMouseCursor_None &&
        !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
      ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
          ->AddRect(TileToScreen(ImVec2(rx, ry)),
                    TileToScreen(ImVec2(rx + 1, ry + 1)), 0xddffffff, 0, 0,
                    3.f);
      std::string coord =
          fmt::format("Screen: {},{}\n  Tile: {},{}", x, y, rx, ry);
      if (fg && bg)
        coord += fmt::format("\n  ID: {},{}", fg->id, bg->id);
      ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
      ImGui::SetTooltip(coord.c_str());
    }
  }
}

void UI::Draw() {
  if (options["ui_viewports"].value) {
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  } else {
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
  }

  doWarp = false;

  ImGuiIO &io = ImGui::GetIO();
  ImGuiContext &g = *GImGui;

  if (screenShotIndex > -1 &&
      screenShotIndex < screenShotRange.x * screenShotRange.y) {
    write_mem_recoverable("render_hud", get_address("render_hud"), "EB 74"_gh,
                          true);
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    int f = screenShotFrame % 5;
    screenShotFrame++;
    if (f == 0) {
      // Max::get().player_room()->y++;
    } else if (f == 2) {
      SaveScreenShot(screenShotFileName + "_" +
                     fmt::format("{:03d}", screenShotIndex + 1) + "_" +
                     TimestampFile());
      if (screenShotIndex + 1 >= screenShotRange.x * screenShotRange.y) {
        screenShotIndex = -1;
        recover_mem("warp");
        *Max::get().player_room() = *Max::get().warp_room();
      }
    } else if (f == 3) {
      Max::get().warp_room()->x++;
      if ((screenShotIndex + 1) % screenShotRange.x == 0) {
        Max::get().warp_room()->x -= screenShotRange.x;
        Max::get().warp_room()->y++;
      }
      write_mem_recoverable("warp", get_address("warp"), "EB"_gh, true);
      screenShotIndex++;
    } else if (f == 4) {
      recover_mem("warp");
    }
    return;
  }

  if (screenShotThisFrame != "") {
    SaveScreenShot(screenShotThisFrame);
    screenShotThisFrame = "";
  }
  if (screenShotNextFrame != "") {
    screenShotThisFrame = screenShotNextFrame;
    screenShotNextFrame = "";
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    return;
  }

  auto ui_scaling = options["ui_scaling"].value; // cache value in case it gets changed
  if (ui_scaling) {
    uiScale = dpiScale;
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    auto &style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImTrunc(style.WindowPadding * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,
                        ImTrunc(style.WindowRounding * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize,
                        ImTrunc(style.WindowMinSize * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImTrunc(style.FramePadding * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,
                        ImTrunc(style.CellPadding * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,
                        ImTrunc(style.IndentSpacing * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize,
                        ImTrunc(style.ScrollbarSize * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize,
                        ImTrunc(style.GrabMinSize * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_SeparatorTextPadding,
                        ImTrunc(style.SeparatorTextPadding * uiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_DockingSeparatorSize,
                        ImTrunc(style.DockingSeparatorSize * uiScale));
  } else {
    uiScale = 1.0f;
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
  }

  Keys();
  Play();
  Windows();
  HUD();
  Cheats();

  ImGui::PopFont();
  if (ui_scaling)
    ImGui::PopStyleVar(10);

  if (ImGui::GetFrameCount() == 20)
    ScaleWindow();
}

void UI::NewWindow(std::string title, ImGuiKeyChord key, ImGuiWindowFlags flags,
                   std::function<void()> cb) {
  windows.push_back(new Window{title, key, flags, cb});
}

void UI::Tooltip(std::string text) {
  if (options["ui_tooltips"].value && ImGui::IsItemHovered()) {
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
    ImGui::SetTooltip(text.c_str());
  }
}

bool UI::Block() {
  ImGuiIO &io = ImGui::GetIO();
  return io.WantCaptureKeyboard || ImGui::GetFrameCount() < lastMenuFrame + 5;
}

void UI::CreateMap() {
  auto *raw_map = (uint8_t *)Max::get().minimap();
  if (raw_map == NULL)
    return;

  int image_width = 800;
  int image_height = 528;
  int length = image_width * image_height * 4;

  int i = 0;
  if (options["map_reveal"].value) {
    do {
      raw_map[i + 3] = 0xf;
      i += 4;
    } while (i < length);
  }

  memcpy(minimap, raw_map, length);

  auto *bits = Max::get().map_bits(2);

  i = 0;
  do {
    if (minimap[i + 3] == 0xf)
      minimap[i + 3] = 0xff;
    else
      minimap[i + 3] = options["map_show"].value ? 0x40 : minimap[i + 3];

    int b = i / 4;
    if (options["map_holes"].value && bits->test(b)) {
      minimap[i] = 0xff;
      minimap[i + 2] = 0xff;
      minimap[i + 3] = 0xff;
    }

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

void UI::SaveINI() {
  CreateDirectory(L"MAXWELL\\Mods", NULL);
  std::string file = "MAXWELL\\MAXWELL.ini";
  std::ofstream writeData(file);
  writeData << "# MAXWELL options" << std::endl;

  writeData << "\n[options] # 0 or 1 unless stated otherwise\n";
  for (const auto &[name, opt] : options) {
    writeData << name << " = " << std::dec << opt.value << std::endl;
  }
  writeData << "scale = " << std::dec << windowScale << " # int, 1 - 10"
            << std::endl;
  writeData.close();
}

void UI::LoadINI() {
  std::string file = "MAXWELL\\MAXWELL.ini";
  toml::value data;
  try {
    data = toml::parse(file);
  } catch (std::exception &) {
    SaveINI();
    return;
  }

  toml::value opts;
  try {
    opts = toml::find(data, "options");
  } catch (std::exception &) {
    SaveINI();
    return;
  }
  for (const auto &[name, opt] : options) {
    options[name].value = (bool)toml::find_or<int>(opts, name, (int)opt.value);
  }
  windowScale = toml::find_or<int>(opts, "scale", 4);

  /*if (options["ui_viewports"].value) {
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  } else {
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
  }*/

  SaveINI();
}

void UI::SaveScreenShot(std::string name) {
  // Get the device and command queue
  // pDevice,
  ID3D12Device *pDevice = pD3DDevice;
  HRESULT hr = pSwapChain->GetDevice(__uuidof(ID3D12Device), (void **)&pDevice);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to get device" << std::endl;
    return;
  }

  ID3D12CommandQueue *pCommandQueue = nullptr;
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  hr = pDevice->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue),
                                   (void **)&pCommandQueue);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to create command queue"
              << std::endl;
    return;
  }

  // Obtain the back buffer
  ID3D12Resource *pBackBuffer = nullptr;
  hr =
      pSwapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void **)&pBackBuffer);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to get back buffer" << std::endl;
    pDevice->Release();
    pCommandQueue->Release();
    return;
  }

  // Describe and create the readback buffer
  D3D12_RESOURCE_DESC desc = pBackBuffer->GetDesc();
  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_READBACK;
  heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

  D3D12_RESOURCE_DESC readbackBufferDesc = {};
  readbackBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  readbackBufferDesc.Alignment = 0;
  readbackBufferDesc.Width =
      desc.Width * desc.Height * 4; // assuming 4 bytes per pixel (RGBA)
  readbackBufferDesc.Height = 1;
  readbackBufferDesc.DepthOrArraySize = 1;
  readbackBufferDesc.MipLevels = 1;
  readbackBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
  readbackBufferDesc.SampleDesc.Count = 1;
  readbackBufferDesc.SampleDesc.Quality = 0;
  readbackBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  readbackBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  ID3D12Resource *pReadbackBuffer = nullptr;
  hr = pDevice->CreateCommittedResource(
      &heapProps, D3D12_HEAP_FLAG_NONE, &readbackBufferDesc,
      D3D12_RESOURCE_STATE_COPY_DEST, nullptr, __uuidof(ID3D12Resource),
      (void **)&pReadbackBuffer);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to create readback buffer"
              << std::endl;
    pBackBuffer->Release();
    pDevice->Release();
    pCommandQueue->Release();
    return;
  }

  // Create command allocator and command list
  ID3D12CommandAllocator *pCommandAllocator = nullptr;
  hr = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                       __uuidof(ID3D12CommandAllocator),
                                       (void **)&pCommandAllocator);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to create command allocator"
              << std::endl;
    pReadbackBuffer->Release();
    pBackBuffer->Release();
    pDevice->Release();
    pCommandQueue->Release();
    return;
  }

  ID3D12GraphicsCommandList *pCommandList = nullptr;
  hr = pDevice->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator, nullptr,
      __uuidof(ID3D12GraphicsCommandList), (void **)&pCommandList);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to create command list"
              << std::endl;
    pCommandAllocator->Release();
    pReadbackBuffer->Release();
    pBackBuffer->Release();
    pDevice->Release();
    pCommandQueue->Release();
    return;
  }

  // Transition the back buffer to the copy source state
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = pBackBuffer;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  pCommandList->ResourceBarrier(1, &barrier);

  // Copy the back buffer to the readback buffer
  D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
  srcLocation.pResource = pBackBuffer;
  srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  srcLocation.SubresourceIndex = 0;

  D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
  dstLocation.pResource = pReadbackBuffer;
  dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  pDevice->GetCopyableFootprints(&desc, 0, 1, 0, &dstLocation.PlacedFootprint,
                                 nullptr, nullptr, nullptr);

  pCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

  // Transition the back buffer back to the present state
  std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
  pCommandList->ResourceBarrier(1, &barrier);

  // Close and execute the command list
  // SOME GAMES ARE THROWING E_INVALIDARG (0x80070057) HERE
  hr = pCommandList->Close();
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to close command list" << std::hex
              << hr << std::endl;
    pCommandList->Release();
    pCommandAllocator->Release();
    pReadbackBuffer->Release();
    pBackBuffer->Release();
    pDevice->Release();
    pCommandQueue->Release();
    return;
  }

  ID3D12CommandList *ppCommandLists[] = {pCommandList};
  pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // Wait for the GPU to finish executing the commands
  ID3D12Fence *pFence = nullptr;
  hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence),
                            (void **)&pFence);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to create fence" << std::endl;
    pCommandList->Release();
    pCommandAllocator->Release();
    pReadbackBuffer->Release();
    pBackBuffer->Release();
    pDevice->Release();
    pCommandQueue->Release();
    return;
  }

  HANDLE hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  pCommandQueue->Signal(pFence, 1);
  pFence->SetEventOnCompletion(1, hEvent);

  if (hEvent != NULL) {
    WaitForSingleObject(hEvent, INFINITE);
    CloseHandle(hEvent);
  }

  pFence->Release();

  // Map the readback buffer
  void *pData;
  hr = pReadbackBuffer->Map(0, nullptr, &pData);
  if (FAILED(hr)) {
    std::cout << "[D3D12-SCREENSHOT] - Failed to map readback buffer"
              << std::endl;
    pCommandList->Release();
    pCommandAllocator->Release();
    pReadbackBuffer->Release();
    pBackBuffer->Release();
    pDevice->Release();
    pCommandQueue->Release();
    return;
  }

  if (CreateDirectory(L"MAXWELL", NULL) ||
      ERROR_ALREADY_EXISTS == GetLastError()) {
    if (CreateDirectory(L"MAXWELL\\Screenshots", NULL) ||
        ERROR_ALREADY_EXISTS == GetLastError()) {
      name = "MAXWELL\\Screenshots\\" + name + ".png";
      Image::save_png_from_data(name, pData, desc.Width, desc.Height);
    }
  }

  // Unmap and release resources
  pReadbackBuffer->Unmap(0, nullptr);
  pCommandList->Release();
  pCommandAllocator->Release();
  pReadbackBuffer->Release();
  pBackBuffer->Release();
  pDevice->Release();
  pCommandQueue->Release();

  std::cout << "[D3D12-SCREENSHOT] - Screenshot saved successfully"
            << std::endl;
}
