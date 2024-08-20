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
const ImU8 u8_zero = 0, u8_one = 1, u8_fifty = 50, u8_min = 0, u8_max = 255,
           u8_five = 5;
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
    "",     "Firecrackers", "Flute",    "Lantern", "Top",
    "Disc", "Bubble Wand",  "Yoyo",     "Slink",   "Remote",
    "Ball", "Wheel",        "UV Light",
};

std::array item_names{
    "Mock Disc",  "Snake Medal", "Cake",      "House key",
    "Office key", "Closet key",  "Eel Medal", "Fanny Pack",
};

std::array misc_names{
    "House opened",
    "Office opened",
    "Closet opened",
    "",
    "",
    "",
    "",
    "",

    "Switch state",
    "Map collected",
    "Stamps collected",
    "Pencil collected",
    "Chameleon defeated",
    "C.Ring collected",
    "Eaten by chameleon",
    "Snake Medal inserted",

    "Eel Medal inserted",
    "Wings acquired",
    "Woke up",
    "B.B.Wand upgrade",
    "65th Egg acquired",
    "All candles lit",
    "Torus active",
    "65th Egg placed",

    "Bat defeated",
    "Ostrich freed",
    "Ostrich defeated",
    "Eel fight active",
    "Eel defeated",
    "No disc in shrine",
    "No disc in statue",
    "",
};

std::array egg_names{
    "Reference Egg",
    "Brown Egg",
    "Raw Egg",
    "Pickled Egg",
    "Big Egg",
    "Swan Egg",
    "Forbidden Egg",
    "Shadow Egg",
    "Vanity Egg",
    "Egg As A Service",
    "Depraved Egg",
    "Chaos Egg",
    "Upside Down Egg",
    "Evil Egg",
    "Sweet Egg",
    "Chocolate Egg",
    "Value Egg",
    "Plant Egg",
    "Red Egg",
    "Orange Egg",
    "Sour Egg",
    "Post Modern Egg",
    "Universal Basic Egg",
    "Laissez-faire Egg",
    "Zen Egg",
    "Future Egg",
    "Friendship Egg",
    "Truth Egg",
    "Transcendental Egg",
    "Ancient Egg",
    "Magic Egg",
    "Mystic Egg",
    "Holiday Egg",
    "Rain Egg",
    "Razzle Egg",
    "Dazzle Egg",
    "Virtual Egg",
    "Normal Egg",
    "Great Egg",
    "Gorgeous Egg",
    "Planet Egg",
    "Moon Egg",
    "Galaxy Egg",
    "Sunset Egg",
    "Goodnight Egg",
    "Dream Egg",
    "Travel Egg",
    "Promise Egg",
    "Ice Egg",
    "Fire Egg",
    "Bubble Egg",
    "Desert Egg",
    "Clover Egg",
    "Brick Egg",
    "Neon Egg",
    "Iridescent Egg",
    "Rust Egg",
    "Scarlet Egg",
    "Sapphire Egg",
    "Ruby Egg",
    "Jade Egg",
    "Obsidian Egg",
    "Crystal Egg",
    "Golden Egg",
};

std::array bunny_names{
    "Tutorial Bunny",
    "Illegal 1",
    "Origami Bunny",
    "Spike Room Bunny",
    "Ghost Bunny",
    "Illegal 2",
    "Fish Mural Bunny",
    "Map Numbers Bunny",
    "TV Bunny",
    "UV Bunny",
    "Bulb Bunny",
    "Chinchilla Bunny",
    "Illegal 3",
    "Illegal 4",
    "Illegal 5",
    "Bunny Mural Bunny",
    "Illegal 6",
    "Illegal 7",
    "Illegal 8",
    "Illegal 9",
    "Illegal 10",
    "Illegal 11",
    "Duck Bunny",
    "Illegal 12",
    "Illegal 13",
    "Ghost Dog Bunny",
    "Illegal 14",
    "Illegal 15",
    "Dream Bunny",
    "Illegal 16",
    "Floor Is Lava Bunny",
    "Crow Bunny",
};

std::array portal_names{
    "Eel", "Frog", "Fish", "Bear", "Dog", "Bird", "Squirrel", "Hippo",
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
bool IsKeyChordDown(ImGuiKeyChord key_chord) {
  ImGuiContext &g = *GImGui;
  key_chord = FixupKeyChord(key_chord);
  ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
  if (g.IO.KeyMods != mods)
    return false;
  ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
  if (key == ImGuiKey_None)
    key = ConvertSingleModFlagToKey(mods);
  if (!IsKeyDown(key))
    return false;
  return true;
}
bool IsKeyChordReleased(ImGuiKeyChord key_chord) {
  ImGuiContext &g = *GImGui;
  key_chord = FixupKeyChord(key_chord);
  ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
  ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
  if (key == ImGuiKey_None)
    key = ConvertSingleModFlagToKey(mods);
  return IsKeyReleased(key) || IsKeyReleased(mods);
}
} // namespace ImGui

template <std::size_t SIZE, typename T>
void Flags(const std::array<const char *, SIZE> names_array, T *flag_field,
           bool show_number = false, int first = 0) {
  for (int idx{first}; idx < SIZE && idx < sizeof(T) * 8; ++idx) {
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

template <typename T>
void UnnamedFlags(const char *name, T *flag_field, int num, int offset = 0) {
  for (int idx{0}; idx < num; ++idx) {
    T value = (T)std::pow(2, idx);
    bool on = (*flag_field & value) == value;

    if (ImGui::Checkbox(fmt::format("{} {}", name, idx + 1 + offset).c_str(),
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
  if (ImGui::Button("Save game##SaveGame")) {
    *Max::get().spawn_room() = *Max::get().player_room();
    Max::get().save_game();
  }
  Tooltip("This sets your current room as spawn and runs the save function "
          "anywhere.\nIn rooms without a phone you will spawn near the "
          "top left corner.");
  if (ImGui::CollapsingHeader("Equipment##PlayerEquipment")) {
    bool disc = *Max::get().equipment() & (1 << 5);
    bool all = (*Max::get().equipment() & 0x1FFE) == 0x1FFE;
    if (ImGui::Checkbox("Unlock all equipment##UnlockAllEquipment", &all)) {
      if (all)
        *Max::get().equipment() = 0x1FFE;
      else
        *Max::get().equipment() = 0;
    }
    ImGui::Separator();
    Flags(equipment_names, Max::get().equipment());
    if (!disc && *Max::get().equipment() & (1 << 5) &&
        (*Max::get().upgrades() & 0x60000000) == 0)
      *Max::get().upgrades() |= 0x20000000;
  }
  if (ImGui::CollapsingHeader("Items##PlayerItems")) {
    bool all = *Max::get().items() == 0xFF;
    if (ImGui::Checkbox("Unlock all items##UnlockAllItems", &all)) {
      if (all) {
        *Max::get().items() = 0xFF;
        *Max::get().shards() = 2;
        *(Max::get().shards() + 12) = 2;
        *(Max::get().shards() + 24) = 2;
      } else {
        *Max::get().items() = 0;
        *Max::get().shards() = 0;
        *(Max::get().shards() + 12) = 0;
        *(Max::get().shards() + 24) = 0;
      }
    }
    ImGui::Separator();
    Flags(item_names, Max::get().items());
    bool shards[3];
    shards[0] = *Max::get().shards();
    shards[1] = *(Max::get().shards() + 12);
    shards[2] = *(Max::get().shards() + 24);
    if (ImGui::Checkbox("##Shard1", &shards[0]))
      *Max::get().shards() = shards[0] * 2;
    ImGui::SameLine(0, 4);
    if (ImGui::Checkbox("##Shard2", &shards[1]))
      *(Max::get().shards() + 12) = shards[1] * 2;
    ImGui::SameLine(0, 4);
    if (ImGui::Checkbox("Kangaroo shards##Shard3", &shards[2]))
      *(Max::get().shards() + 24) = shards[2] * 2;
  }
  if (ImGui::CollapsingHeader("Miscellaneous##PlayerMisc"))
    Flags(misc_names, Max::get().upgrades());
  if (ImGui::CollapsingHeader("Eggs##PlayerEggs")) {
    bool all = *Max::get().eggs() == 0xFFFFFFFFFFFFFFFF;
    if (ImGui::Checkbox("Unlock all eggs##UnlockAllEggs", &all)) {
      if (all) {
        *Max::get().eggs() = 0xFFFFFFFFFFFFFFFF;
        *Max::get().upgrades() |= (1 << 20);
      } else {
        *Max::get().eggs() = 0;
        *Max::get().upgrades() &= ~(1 << 20);
      }
    }
    ImGui::Separator();
    Flags(egg_names, Max::get().eggs());
    ImGui::CheckboxFlags("65th Egg", Max::get().upgrades(), 1 << 20);
  }
  if (ImGui::CollapsingHeader("Bunnies##PlayerBunnies")) {
    bool all = *Max::get().bunnies() == 0xD2408FDD;
    if (ImGui::Checkbox("Unlock legal bunnies##UnlockLegalBunnies", &all)) {
      if (all)
        *Max::get().bunnies() = 0xD2408FDD;
      else
        *Max::get().bunnies() = 0;
    }
    ImGui::Separator();
    Flags(bunny_names, Max::get().bunnies());
  }
  if (ImGui::CollapsingHeader("Candles##PlayerCandles")) {
    ImGui::TextWrapped("Only the first 9 candles exist on a vanilla map.");
    bool all = (*Max::get().candles() & 0x1FF) == 0x1FF;
    if (ImGui::Checkbox("Unlock legal candles##UnlockAllCandles", &all)) {
      if (all) {
        *Max::get().candles() = 0x1FF;
      } else {
        *Max::get().candles() = 0;
      }
    }
    ImGui::Separator();
    UnnamedFlags("Candle", Max::get().candles(), 16);
  }
  if (ImGui::CollapsingHeader("Chests##PlayerChests")) {
    ImGui::TextWrapped("Only the first 102 chests exist on a vanilla map.");
    UnnamedFlags("Chest", Max::get().chests(), 64);
    UnnamedFlags("Chest", Max::get().chests() + 1, 64, 64);
  }
  if (ImGui::CollapsingHeader("Flames##PlayerFlames")) {
    bool all = *(uint32_t *)Max::get().flames() == 0x05050505;
    if (ImGui::Checkbox("Place all flames##UnlockAllFlames", &all)) {
      for (int i = 0; i < 4; ++i)
        if (all) {
          *(Max::get().flames() + i) = 5;
        } else {
          *(Max::get().flames() + i) = 0;
        }
    }
    ImGui::Separator();
    ImGui::SliderScalar("Blue Flame / Seahorse##BlueFlameSlider",
                        ImGuiDataType_U8, Max::get().flames(), &u8_zero,
                        &u8_five);
    ImGui::SliderScalar("Purple Flame / Dog##PurpleFlameSlider",
                        ImGuiDataType_U8, Max::get().flames() + 1, &u8_zero,
                        &u8_five);
    ImGui::SliderScalar("Violet Flame / Chameleon##VioletFlameSlider",
                        ImGuiDataType_U8, Max::get().flames() + 2, &u8_zero,
                        &u8_five);
    ImGui::SliderScalar("Green Flame / Ostrich##GreenFlameSlider",
                        ImGuiDataType_U8, Max::get().flames() + 3, &u8_zero,
                        &u8_five);
  }
  if (ImGui::CollapsingHeader("Animal head portals##PlayerPortals")) {
    bool all = (*Max::get().portals() & 0xfe) == 0xfe;
    if (ImGui::Checkbox("Unlock all portals##UnlockAllPortals", &all)) {
      if (all) {
        *Max::get().portals() = 0xfe;
        *(Max::get().portals() + 1) = 0xfe;
        *Max::get().upgrades() &= ~(1 << 27);
        *Max::get().upgrades() |= (1 << 28);
      } else {
        *Max::get().portals() = 0;
        *(Max::get().portals() + 1) = 0;
        *Max::get().upgrades() &= ~(1 << 27);
        *Max::get().upgrades() &= ~(1 << 28);
      }
    }
    ImGui::SeparatorText("Heads seen");
    ImGui::PushID("AnimalHeadsSeen");
    ImGui::CheckboxFlags("Eel fight active", Max::get().upgrades(), 1 << 27);
    Flags(portal_names, Max::get().portals(), false, 1);
    ImGui::PopID();
    ImGui::PushID("AnimalHeadsUnlocked");
    ImGui::SeparatorText("Heads unlocked");
    ImGui::CheckboxFlags("Eel", Max::get().upgrades(), 1 << 28);
    Flags(portal_names, Max::get().portals() + 1, false, 1);
    ImGui::PopID();
  }
  if (*Max::get().upgrades() & (1 << 28))
    *Max::get().upgrades() &= ~(1 << 27);
  if (ImGui::CollapsingHeader("Consumables##PlayerConsumables")) {
    bool all = false;
    ImGui::Checkbox("Max out stats##UnlockMaxStats",
                    &options["cheat_stats"].value);
    ImGui::DragScalar("Health##PlayerHealth", ImGuiDataType_S8,
                      Max::get().player_hp(), 0.1f);
    ImGui::DragScalar("More health##PlayerMoreHealth", ImGuiDataType_S8,
                      Max::get().player_hp() + 1, 0.1f);
    ImGui::DragScalar("Keys##PlayerKeys", ImGuiDataType_U8, Max::get().keys(),
                      0.1f);
    ImGui::DragScalar("Matches##PlayerMatches", ImGuiDataType_U8,
                      Max::get().keys() + 1, 0.1f);
    ImGui::DragScalar("Firecrackers##PlayerFirecrackers", ImGuiDataType_U8,
                      Max::get().keys() + 2, 0.1f);
  }
  if (ImGui::CollapsingHeader("Position##PlayerPositionAndRoom")) {
    ImGui::InputInt2("Room##PlayerRoom", &Max::get().player_room()->x);
    ImGui::InputFloat2("Position##PlayerPosition",
                       &Max::get().player_position()->x);
    ImGui::InputFloat2("Velocity##PlayerVelocity",
                       &Max::get().player_velocity()->x);
    ImGui::InputInt2("Spawn room##PlayerSpawnRoom",
                     &Max::get().spawn_room()->x);
    ImGui::InputInt2("Respawn room##PlayerRespawnRoom",
                     &Max::get().respawn_room()->x);
    ImGui::InputInt2("Respawn tile##PlayerRespawnTile",
                     &Max::get().respawn_position()->x);
    ImGui::InputInt("Map##PlayerMap", Max::get().player_map());
    ImGui::InputFloat2("Wheel##PlayerWheelPosition",
                       &Max::get().player_wheel()->x);
  }
  if (ImGui::CollapsingHeader("State##PlayerAndGameState")) {
    ImGui::InputScalar("State##PlayerState", ImGuiDataType_U8,
                       Max::get().player_state());
    ImGui::InputScalar("Flute##PlayerFluteDir", ImGuiDataType_U8,
                       Max::get().player_flute());
    ImGui::InputScalar("Item##PlayerCurrentItem", ImGuiDataType_U8,
                       Max::get().item());
    ImGui::InputScalar("In-game time##StateIngameTime", ImGuiDataType_U32,
                       Max::get().timer());
    ImGui::InputScalar("Total time##StateTotalTime", ImGuiDataType_U32,
                       Max::get().timer() + 1);
    ImGui::Checkbox("Paused##StatePaused", &Max::get().pause()->paused);
  }
  if (ImGui::CollapsingHeader("Warp##PlayerWarp")) {
    ImGui::InputInt2("Warp room##PlayerWarpRoom", &Max::get().warp_room()->x);
    ImGui::InputInt2("Warp position##PlayerWarpPosition",
                     &Max::get().warp_position()->x);
    ImGui::InputInt("Warp map##PlayerWarpMap", Max::get().warp_map());
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

  ImVec2 realmapsize{800 * uiScale * mapScale, 528 * uiScale * mapScale};
  ImVec2 roomsize{40 * uiScale * mapScale, 22 * uiScale * mapScale};

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
  ImGui::SameLine(mapsize.x - 60.f * uiScale * mapScale);
  if (ImGui::Button(
          "Refresh##MinimapRefresh",
          ImVec2(60.f * uiScale * mapScale + ImGui::GetStyle().WindowPadding.x,
                 ImGui::GetItemRectSize().y)) ||
      (((options["map_auto"].value ||
         ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_warp"])) &&
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
      if (ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_warp"])) {
        *Max::get().player_state() = 18;
        *Max::get().warp_room() = wroom;
        *Max::get().warp_position() = wpos;
        *Max::get().warp_map() = layer;
        doWarp = true;
      } else if (ImGui::IsKeyChordReleased((ImGuiKey)keys["mouse_warp"]) &&
                 *Max::get().player_state() == 18) {
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
                ((Max::get().player_position()->x + 4.f) / 320.f * roomsize.x);
      auto py = Max::get().player_room()->y * roomsize.y +
                ((Max::get().player_position()->y + 4.f) / 180.f * roomsize.y);
      ImGui::GetWindowDrawList()->AddCircleFilled(
          ImVec2(a.x + d.x + px - c.x - bordersize.x,
                 a.y + d.y + py - c.y - bordersize.y),
          3.f * uiScale * mapScale, 0xee0000ee);
    }

    if (options["map_wheel"].value) {
      auto px = Max::get().player_room()->x * roomsize.x +
                (Max::get().player_wheel()->x / 320.f * roomsize.x);
      auto py = Max::get().player_room()->y * roomsize.y +
                (Max::get().player_wheel()->y / 180.f * roomsize.y);
      while (px < 80.f * uiScale * mapScale)
        px += 640.f * uiScale * mapScale;
      while (px > 720.f * uiScale * mapScale)
        px -= 640.f * uiScale * mapScale;
      while (py > 440.f * uiScale * mapScale)
        py -= 352.f * uiScale * mapScale;
      ImGui::GetWindowDrawList()->AddCircle(
          ImVec2(a.x + d.x + px - c.x - bordersize.x,
                 a.y + d.y + py - c.y - bordersize.y),
          4.f * uiScale * mapScale, 0xee00ffee, 0, 1.5f * uiScale * mapScale);
    }

    if (options["map_uv_bunny"].value &&
        (*Max::get().bunnies() & (1 << 9)) == 0) {
      auto px = ((Max::get().uv_bunny()->x + 16.f) / 320.f * roomsize.x);
      auto py = ((Max::get().uv_bunny()->y + 48.f) / 180.f * roomsize.y);
      ImGui::GetWindowDrawList()->AddCircleFilled(
          ImVec2(a.x + d.x + px - c.x - bordersize.x,
                 a.y + d.y + py - c.y - bordersize.y),
          4.f * uiScale * mapScale, rand() | 0xff000000);
    }

    for (auto &tile : searchTiles) {
      auto px =
          tile.room.x * roomsize.x + (tile.pos.x * 8.f / 320.f * roomsize.x);
      auto py =
          tile.room.y * roomsize.y + (tile.pos.y * 8.f / 180.f * roomsize.y);
      ImGui::GetWindowDrawList()->AddRectFilled(
          ImVec2(a.x + d.x + px - c.x - bordersize.x - uiScale * mapScale,
                 a.y + d.y + py - c.y - bordersize.y - uiScale * mapScale),
          ImVec2(a.x + d.x + px - c.x - bordersize.x + uiScale * mapScale,
                 a.y + d.y + py - c.y - bordersize.y + uiScale * mapScale),
          0xffffff33);
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

void UI::UpdateOptions() {
  if (options["cheat_palette"].value && CheatsEnabled()) {
    Max::get().force_palette = forcedPalette;
  } else {
    Max::get().force_palette = std::nullopt;
  }
  Max::get().use_keymap = options["input_custom"].value;
  Max::get().use_igt = options["cheat_igt"].value && CheatsEnabled();
}

uint8_t AnyKey() {
  for (int i = 8; i < 255; ++i)
    if (ImGui::GetIO().KeysDown[i])
      return i;
  return 0;
}

void UI::KeyCapture() {
  ImGuiIO &io = ImGui::GetIO();
  io.WantCaptureKeyboard = true;
  auto base = ImGui::GetMainViewport();
  ImGui::SetNextWindowSize(base->Size);
  ImGui::SetNextWindowPos(base->Pos);
  ImGui::SetNextWindowViewport(base->ID);
  ImGui::SetNextWindowBgAlpha(0.75);
  ImGui::Begin("KeyCapture", NULL,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus |
                   ImGuiWindowFlags_NoNavInputs |
                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking);
  ImGui::InvisibleButton("KeyCaptureCanvas", ImGui::GetContentRegionMax(),
                         ImGuiButtonFlags_MouseButtonLeft |
                             ImGuiButtonFlags_MouseButtonRight);
  ImDrawList *dl = ImGui::GetForegroundDrawList();
  std::string buf =
      fmt::format("Enter new key/button combo for {}.\nModifiers Ctrl, Alt "
                  "and Shift are available.",
                  key_to_change);
  ImVec2 textsize = ImGui::CalcTextSize(buf.c_str());
  dl->AddText({base->Pos.x + base->Size.x / 2 - textsize.x / 2,
               base->Pos.y + base->Size.y / 2 - textsize.y / 2},
              ImColor(1.0f, 1.0f, 1.0f, .8f), buf.c_str());
  for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; ++i) {
    if (ImGui::IsKeyReleased((ImGuiKey)i)) {
      ImGuiKeyChord key_chord = (ImGuiKeyChord)i;
      ImGuiKey mods = (ImGuiKey)io.KeyMods;
      key_chord |= mods;
      keys[key_to_change] = (ImGuiKeyChord)key_chord;
      key_to_change = "";
    }
  }
  ImGui::End();
}

std::string GetKeyName(unsigned int virtualKey) {
  unsigned int scanCode = MapVirtualKeyA(virtualKey, MAPVK_VK_TO_VSC);

  // because MapVirtualKey strips the extended bit for some keys
  switch (virtualKey) {
  case VK_LEFT:
  case VK_UP:
  case VK_RIGHT:
  case VK_DOWN: // arrow keys
  case VK_PRIOR:
  case VK_NEXT: // page up and page down
  case VK_END:
  case VK_HOME:
  case VK_INSERT:
  case VK_DELETE:
  case VK_DIVIDE: // numpad slash
  case VK_NUMLOCK: {
    scanCode |= 0x100; // set extended bit
    break;
  }
  }

  char keyName[50];
  if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName)) != 0) {
    return keyName;
  } else {
    return "???";
  }
}

void UI::DrawCustomKey(std::string name, GAME_INPUT i) {
  ImGuiIO &io = ImGui::GetIO();
  auto key = Max::get().keymap[i];
  ImGui::PushID(name.c_str());
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::Text("%s", name.c_str());
  ImGui::TableNextColumn();
  ImGui::Text("%s", GetKeyName(key));
  ImGui::TableNextColumn();
  ImGui::InputScalar(
      "##GameKeyCode", ImGuiDataType_U8, &key, NULL, NULL, "0x%x",
      ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_AllowTabInput);
  if (ImGui::IsItemActive() && !ImGui::IsKeyReleased(ImGuiKey_MouseLeft)) {
    key = AnyKey();
    if (key) {
      Max::get().keymap[i] = key;
      ImGui::ClearActiveID();
      SaveINI();
    }
  }
  ImGui::TableNextColumn();
  if (ImGui::Button("Unset")) {
    Max::get().keymap[i] = 0;
    SaveINI();
  }
  ImGui::PopID();
}

void UI::DrawUIKeys() {
  ImGuiIO &io = ImGui::GetIO();
  ImGui::PushID("UIKeys");
  ImGui::BeginTable("##UIKeysTable", 4);
  ImGui::TableSetupColumn("Tool");
  ImGui::TableSetupColumn("Keys");
  ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed,
                          60.f * uiScale);
  ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                          135.f * uiScale);
  ImGui::TableHeadersRow();
  for (auto &[name, key] : keys) {
    ImGui::PushID(name.c_str());
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", name.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", ImGui::GetKeyChordName(key));
    ImGui::TableNextColumn();
    ImGui::InputScalar("##UIKeyCode", ImGuiDataType_U8, &key, NULL, NULL,
                       "0x%x", ImGuiInputTextFlags_ReadOnly);
    ImGui::TableNextColumn();
    if (ImGui::Button("Set")) {
      key_to_change = name;
      SaveINI();
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button("Unset")) {
      key = 0;
      SaveINI();
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button("Reset")) {
      key = default_keys[name];
      SaveINI();
    }
    ImGui::PopID();
  }
  ImGui::EndTable();
  ImGui::PopID();
  ImGui::TextUnformatted(
      "Click Set and press any key to change.\nModifiers "
      "Ctrl, Alt and Shift are available.\nMouse controls can also "
      "be bound to keys.");
}

void UI::DrawOptions() {
  ImGuiIO &io = ImGui::GetIO();
  ImGui::PushItemWidth(120.f * uiScale);
  bool noclip = options["cheat_noclip"].value;
  std::string cat;
  for (auto &[name, enabled] : options) {
    std::string newcat = name.substr(0, name.find("_"));
    if (newcat != cat) {
      if (newcat == "cheat")
        ImGui::SeparatorText("Cheats");
      else if (newcat == "map")
        ImGui::SeparatorText("Minimap");
      else if (newcat == "ui")
        ImGui::SeparatorText("User interface");
      else if (newcat == "input")
        ImGui::SeparatorText("Input");
      cat = newcat;
    }
    Option(name);
  }
  if (noclip && !options["cheat_noclip"].value)
    *Max::get().player_state() = 0;
  UpdateOptions();
  ImGui::SliderFloat("Minimap scale", &mapScale, 1.f, 5.f, "%.1fx");
  if (ImGui::SliderInt("Window scale", &windowScale, 1, 10, "%dx")) {
    ScaleWindow();
  }
  ImGui::SliderFloat("Alpha", &ImGui::GetStyle().Alpha, 0.2f, 1.0f, "%.1f");

  if (SubMenu("Game keyboard bindings")) {
    ImGui::PushID("CustomKeys");
    if (ImGui::Checkbox("Use custom keyboard bindings",
                        &options["input_custom"].value))
      Max::get().use_keymap = options["input_custom"].value;
    ImGui::BeginTable("##GameKeysTable", 4);
    ImGui::TableSetupColumn("Action");
    ImGui::TableSetupColumn("Key");
    ImGui::TableSetupColumn("Hex", ImGuiTableColumnFlags_WidthFixed,
                            60.f * uiScale);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed,
                            100.f * uiScale);
    ImGui::TableHeadersRow();
    DrawCustomKey("Up", GAME_INPUT::UP);
    DrawCustomKey("Down", GAME_INPUT::DOWN);
    DrawCustomKey("Left", GAME_INPUT::LEFT);
    DrawCustomKey("Right", GAME_INPUT::RIGHT);
    DrawCustomKey("Jump", GAME_INPUT::JUMP);
    DrawCustomKey("Action/Back", GAME_INPUT::ACTION);
    DrawCustomKey("Item", GAME_INPUT::ITEM);
    DrawCustomKey("Inventory", GAME_INPUT::INVENTORY);
    DrawCustomKey("Map", GAME_INPUT::MAP);
    DrawCustomKey("Previous item", GAME_INPUT::LB);
    DrawCustomKey("Next item", GAME_INPUT::RB);
    DrawCustomKey("Pause", GAME_INPUT::PAUSE);
    DrawCustomKey("HUD", GAME_INPUT::HUD);
    DrawCustomKey("Cring", GAME_INPUT::CRING);
    ImGui::EndTable();
    ImGui::PopID();
    ImGui::TextUnformatted("Click an input field and press any key to change.");
    EndMenu();
  }

  if (SubMenu("UI keyboard bindings")) {
    DrawUIKeys();
    EndMenu();
  }

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
  /*if (inMenu)
    ret = ImGui::MenuItem(title.c_str(), "", &options[name].value);
  else*/
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

bool UI::SubMenu(std::string name) {
  if (inMenu)
    return ImGui::BeginMenu(name.c_str());
  else
    return ImGui::CollapsingHeader(name.c_str());
}

void UI::EndMenu() {
  if (inMenu)
    ImGui::EndMenu();
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
    ImGui::InputText("File prefix##ScreenshotPrefix", &screenShotFileName);
    ImGui::InputInt2("Room range##ScreenshotRoomRange", &screenShotRange.x);
    if (ImGui::Button("Capture (.)##ScreenshotCapture"))
      ScreenShot();
    ImGui::SameLine(0, 4);
    if (ImGui::Button("Capture range##ScreenshotCaptureRange")) {
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
    if (ImGui::Checkbox("Enable sequencer##SequencerEnable",
                        &sequencer.enabled)) {
      Max::get().input = PLAYER_INPUT::SKIP;
      Max::get().inputs.clear();
    }
    if (ImGui::Button("Clear queue##SequencerClearQueue")) {
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
      Max::get().render_queue.push_back([&]() {
        Max::get().draw_text_small(57, 17, L"bunny sequencer 0.2");
      });
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
  ImGui::Text("%02d,%02d %02d,%02d %s", tile.room.x, tile.room.y, tile.pos.x,
              tile.pos.y, (tile.layer ? "B" : "F"));
  ImGui::SameLine(ImGui::GetContentRegionMax().x - 24.f * uiScale, 0);
  if (ImGui::Button("Go", ImVec2(24.f * uiScale, ImGui::GetFrameHeight()))) {
    *Max::get().warp_map() = tile.map;
    *Max::get().warp_room() = tile.room;
    Max::get().warp_position()->x = 8 * tile.pos.x;
    Max::get().warp_position()->y = 8 * tile.pos.y;
    doWarp = true;
  }
}

void ColorEdit3(const char *label, uint8_t *col,
                ImGuiColorEditFlags flags = 0) {
  float col4[4]{col[0] / 255.0f, col[1] / 255.0f, col[2] / 255.0f, 1.0f};

  ImGui::ColorEdit4(label, col4, flags | ImGuiColorEditFlags_NoAlpha);

  col[0] = col4[0] * 255.f;
  col[1] = col4[1] * 255.f;
  col[2] = col4[2] * 255.f;
}

void ColorEdit4(const char *label, uint8_t *col,
                ImGuiColorEditFlags flags = 0) {
  float col4[4]{col[0] / 255.0f, col[1] / 255.0f, col[2] / 255.0f,
                col[3] / 255.0f};

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
    ImGui::PushItemWidth(140.f * uiScale);
    static bool lockCurrentRoom{true};
    ImGui::Checkbox("Select current room", &lockCurrentRoom);
    if (lockCurrentRoom) {
      selectedRoom.pos = *Max::get().player_room();
      selectedRoom.map = *Max::get().player_map();
      selectedRoom.room = Max::get().room(selectedRoom.map, selectedRoom.pos.x,
                                          selectedRoom.pos.y);
    }

    ImGui::BeginDisabled(lockCurrentRoom);
    if (ImGui::InputInt2("Position##RoomPosition", &selectedRoom.pos.x)) {
      selectedRoom.room = Max::get().room(selectedRoom.map, selectedRoom.pos.x,
                                          selectedRoom.pos.y);
    }
    ImGui::EndDisabled();

    if (selectedRoom.room) {
      if (!defaultRoom.contains(selectedRoom.room))
        defaultRoom[selectedRoom.room] = {selectedRoom.room->bgId,
                                          selectedRoom.room->waterLevel,
                                          selectedRoom.room->params};
      ImGui::InputScalarN("BG##RoomBG", ImGuiDataType_U8,
                          &selectedRoom.room->bgId, 1, &u8_one);
      ImGui::InputScalarN("Lighting##RoomLighting", ImGuiDataType_U8,
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
      ImGui::Checkbox("Forced lighting", &options["cheat_palette"].value);
      UpdateOptions();
      if (ImGui::Button("Reset room params##ResetRoomParams") &&
          defaultRoom.contains(selectedRoom.room)) {
        selectedRoom.room->bgId = defaultRoom[selectedRoom.room].bgId;
        selectedRoom.room->waterLevel =
            defaultRoom[selectedRoom.room].waterLevel;
        selectedRoom.room->params = defaultRoom[selectedRoom.room].params;
      }

      auto palette = options["cheat_palette"].value
                         ? forcedPalette
                         : selectedRoom.room->params.palette;
      ImGui::SeparatorText(fmt::format("Light params ({})", palette).c_str());

      auto amb = Max::get().lighting(palette);

      if (amb) {
        if (!defaultLighting.contains(palette))
          defaultLighting[palette] = *amb;

        ColorEdit3("Color", amb->ambient_light);
        ColorEdit3("FG tile multiplier", amb->fg_ambient_multi);
        ColorEdit3("BG tile multiplier", amb->bg_ambient_multi);

        ColorEdit4("Lamp intensity", amb->light_intensity);
        ImGui::DragFloat3("Dividers", amb->dividers, 0.1f);
        ImGui::DragFloat("Saturation", &amb->saturation, 0.1f);
        ImGui::DragFloat("BG texture multiplier", &amb->bg_tex_light_multi,
                         0.1f);

        if (ImGui::Button("Reset light params##ResetRoomLightParams") &&
            defaultLighting.contains(palette))
          *amb = defaultLighting[palette];
        ImGui::SameLine(0, 4);
        if (ImGui::Button("Dump lighting##DumpRoomLightParams"))
          Max::get().dump_lighting();
        Tooltip("Edited lighting asset will be dumped\n"
                "to: MAXWELL/Dump/Assets/179.ambient");
      }
    }
    ImGui::PopItemWidth();
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
    ImGui::PushItemWidth(100.f * uiScale);
    static uint16_t searchId{0};
    ImGui::InputScalar("ID", ImGuiDataType_U16, &searchId, &u16_one);
    const bool focused = ImGui::IsItemFocused();
    bool doSearch = false;
    bool doClear = false;
    ImGui::SameLine(130.f * uiScale, 4);
    if (ImGui::Button("Search##SearchTiles") ||
        (focused && ImGui::IsKeyPressed(ImGuiKey_Enter))) {
      doSearch = true;
      doClear = !ImGui::IsKeyDown((ImGuiKey)keys["submit_modifier"]);
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
      std::sort(searchTiles.begin(), searchTiles.end(),
                [](const auto &a, const auto &b) {
                  return (a.room.y < b.room.y) ||
                         (a.room.y == b.room.y && a.room.x < b.room.x) ||
                         (a.room.y == b.room.y && a.room.x == b.room.x &&
                          a.pos.y < b.pos.y) ||
                         (a.room.y == b.room.y && a.room.x == b.room.x &&
                          a.pos.y == b.pos.y && a.pos.x < b.pos.x) ||
                         (a.room.y == b.room.y && a.room.x == b.room.x &&
                          a.pos.y == b.pos.y && a.pos.x == b.pos.x &&
                          a.layer < b.layer);
                });
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
    ImGui::PopItemWidth();
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
  keys = default_keys;
  LoadINI();

  dpiScale = scale;
  if (options["ui_scaling"].value)
    uiScale = dpiScale;

  NewWindow("Player", keys["tool_player"], 0, [this]() { this->DrawPlayer(); });
  NewWindow("Minimap", keys["tool_map"], ImGuiWindowFlags_AlwaysAutoResize,
            [this]() { this->DrawMap(); });
  NewWindow("Tools", keys["tool_tools"], 0, [this]() { this->DrawTools(); });
  NewWindow("Level", keys["tool_level"], 0, [this]() { this->DrawLevel(); });
  NewWindow("Settings", ImGuiKey_None, 0, [this]() { this->DrawOptions(); });
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
  auto ret = true;
  if (ImGui::IsKeyChordPressed(keys["toggle_ui"])) {
    options["ui_visible"].value ^= true;
    SaveINI();
  }
  if (!options["ui_visible"].value && options["ui_ignore"].value)
    return false;
  if (ImGui::IsKeyReleased((ImGuiKey)keys["escape"]))
    ImGui::SetWindowFocus(nullptr);
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
  else if (ImGui::IsKeyChordPressed(keys["toggle_stats"]))
    options["cheat_stats"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_darkness"]))
    options["cheat_darkness"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_lights"]))
    options["cheat_lights"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_clouds"]))
    options["cheat_clouds"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_palette"]))
    options["cheat_palette"].value ^= true;
  else if (ImGui::IsKeyChordPressed(keys["toggle_gameboy"]))
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
  } else if (ImGui::IsKeyChordPressed(keys["skip"], ImGuiInputFlags_Repeat))
    Max::get().skip = true;
  else
    ret = false;
  UpdateOptions();
  return ret;
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

bool UI::CheatsEnabled() {
  return options["cheat_active"].value &&
         (options["ui_visible"].value || !options["ui_ignore_cheats"].value);
}

void UI::Cheats() {
  if (doWarp && get_address("warp") && CheatsEnabled()) {
    Max::get().decrypt_stuff();
    write_mem_recoverable("warp", get_address("warp"), "EB"_gh, true);
  } else {
    recover_mem("warp");
  }

  if (options["input_block"].value && get_address("keyboard") &&
      CheatsEnabled()) {
    if (Block()) {
      write_mem_recoverable("block", get_address("keyboard"), get_nop(6), true);
    } else {
      recover_mem("block");
    }
  } else {
    recover_mem("block");
  }

  if (options["cheat_damage"].value && get_address("damage") &&
      CheatsEnabled()) {
    write_mem_recoverable("damage", get_address("damage"), get_nop(6), true);
  } else {
    recover_mem("damage");
  }

  if (options["cheat_godmode"].value && get_address("god") && CheatsEnabled()) {
    write_mem_recoverable("god", get_address("god"), "E9 71 01 00 00 90"_gh,
                          true);
  } else {
    recover_mem("god");
  }

  if (options["cheat_darkness"].value && get_address("render_darkness") &&
      CheatsEnabled()) {
    write_mem_recoverable("render_darkness", get_address("render_darkness"),
                          "EB 19"_gh, true);
  } else {
    recover_mem("render_darkness");
  }

  if (options["cheat_lights"].value && get_address("render_lights") &&
      CheatsEnabled()) {
    write_mem_recoverable("render_lights", get_address("render_lights"),
                          "E9 8A 00 00 00 90"_gh, true);
  } else {
    recover_mem("render_lights");
  }

  if (options["cheat_gameboy"].value && get_address("render_gameboy") &&
      CheatsEnabled()) {
    write_mem_recoverable("render_gameboy", get_address("render_gameboy"),
                          "EB 0E"_gh, true);
  } else {
    recover_mem("render_gameboy");
  }

  if (options["cheat_clouds"].value && get_address("render_clouds") &&
      CheatsEnabled()) {
    write_mem_recoverable("render_clouds", get_address("render_clouds"),
                          "EB 24"_gh, true);
  } else {
    recover_mem("render_clouds");
  }

  if (options["cheat_hud"].value && get_address("render_hud") &&
      CheatsEnabled()) {
    write_mem_recoverable(
        "render_hud", get_address("render_hud"), "EB 74"_gh,
        true); // jmp over this and the next if that renders some more text
  } else {
    recover_mem("render_hud");
  }

  if (options["cheat_player"].value && get_address("render_player") &&
      CheatsEnabled()) {
    write_mem_recoverable("render_player", get_address("render_player"),
                          "C3"_gh, true);
  } else {
    recover_mem("render_player");
  }

  if (options["cheat_credits"].value && get_address("skip_credits") &&
      CheatsEnabled()) {
    write_mem_recoverable("skip_credits", get_address("skip_credits"),
                          get_nop(2), true);
  } else {
    recover_mem("skip_credits");
  }

  if (options["cheat_noclip"].value && CheatsEnabled()) {
    *Max::get().player_state() = 18;
  }

  if (options["cheat_groundhog"].value && get_address("groundhog_day") &&
      CheatsEnabled()) {
    write_mem_recoverable("groundhog_day", get_address("groundhog_day"),
                          get_nop(2), true);
    write_mem_recoverable("groundhog_day2", get_address("groundhog_day") + 26,
                          get_nop(2), true);
  } else {
    recover_mem("groundhog_day");
    recover_mem("groundhog_day2");
  }

  if (options["cheat_water"].value && get_address("render_water") &&
      CheatsEnabled()) {
    write_mem_recoverable("cheat_water", get_address("render_water"), "EB"_gh,
                          true);
  } else {
    recover_mem("cheat_water");
  }

  if (options["cheat_stats"].value) {
    if (*Max::get().player_hp() < 12)
      *Max::get().player_hp() = 12;
    *(Max::get().player_hp() + 1) = 4;
    *Max::get().keys() = 9;
    *(Max::get().keys() + 1) = 9;
    *(Max::get().keys() + 2) = 255;
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
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {2.f, 2.f});
          window->cb();
          ImGui::PopStyleVar();
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
    std::string version =
        fmt::format("MAXWELL {}", get_version()) + " | GAME " + game_version();
    ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
        ->AddText(ImVec2(io.DisplaySize.x / 2.f -
                             ImGui::CalcTextSize(version.c_str()).x / 2.f +
                             Base().x,
                         io.DisplaySize.y -
                             ImGui::GetTextLineHeightWithSpacing() + Base().y),
                  0x99999999, version.c_str());
    if (ImGui::GetFrameCount() < 600 && !options["ui_visible"].value)
      ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
          ->AddText(ImVec2(io.DisplaySize.x / 2.f -
                               ImGui::CalcTextSize(
                                   "MAXWELL is hidden, press F10 to show")
                                       .x /
                                   2.f +
                               Base().x,
                           io.DisplaySize.y -
                               ImGui::GetTextLineHeightWithSpacing() * 2 +
                               Base().y),
                    0x99999999, "MAXWELL is hidden, press F10 to show");
  }

  if (!options["ui_visible"].value && options["ui_ignore"].value)
    return;

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

  if (options["ui_visible"].value && windowScale > 2) {
    std::string hud;
    if (options["ui_show_cheats"].value)
      hud += fmt::format(
          "CHEATS:{}{}{}{}{}{}{}{}{}{}{}{}{}{} | INPUT:{}{}{}",
          options["cheat_active"].value ? "" : " DISABLED",
          options["cheat_damage"].value && CheatsEnabled() ? " NODAMAGE" : "",
          options["cheat_noclip"].value && CheatsEnabled() ? " NOCLIP" : "",
          options["cheat_godmode"].value && CheatsEnabled() ? " GODMODE" : "",
          options["cheat_darkness"].value && CheatsEnabled() ? " NODARKNESS"
                                                             : "",
          options["cheat_lights"].value && CheatsEnabled() ? " NOLAMPS" : "",
          options["cheat_palette"].value && CheatsEnabled() ? " LIGHTING" : "",
          options["cheat_water"].value && CheatsEnabled() ? " NOWATER" : "",
          options["cheat_clouds"].value && CheatsEnabled() ? " NOCLOUDS" : "",
          options["cheat_hud"].value && CheatsEnabled() ? " NOHUD" : "",
          options["cheat_player"].value && CheatsEnabled() ? " NOBEAN" : "",
          options["cheat_credits"].value && CheatsEnabled() ? " NOCREDITS" : "",
          options["cheat_groundhog"].value && CheatsEnabled() ? " GROUNDHOG"
                                                              : "",
          options["cheat_igt"].value && CheatsEnabled() ? " IGT" : "",

          options["input_block"].value ? " BLOCK" : "",
          options["input_custom"].value ? " CUSTOM" : "",
          options["input_mouse"].value ? " MOUSE" : "");
    if (options["ui_show_datetime"].value) {
      if (options["ui_show_cheats"].value)
        hud += " | ";
      hud += Timestamp();
    }
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
    auto *fg =
        Max::get().tile(*Max::get().player_map(), Max::get().player_room()->x,
                        Max::get().player_room()->y, rx, ry, 0);
    auto *bg =
        Max::get().tile(*Max::get().player_map(), Max::get().player_room()->x,
                        Max::get().player_room()->y, rx, ry, 1);

    if (options["input_mouse"].value) {
      if (ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_warp"]) &&
          !io.WantCaptureMouse) {
        if (inbound || (ImGui::GetFrameCount() % 10) == 0) {
          Max::get().player_position()->x = x - 4;
          Max::get().player_position()->y = y - 4;
        }
        Max::get().player_velocity()->x = 0;
        Max::get().player_velocity()->y = 0;
        *Max::get().player_state() = 18;
      } else if (ImGui::IsKeyChordReleased((ImGuiKey)keys["mouse_warp"]) &&
                 *Max::get().player_state() == 18) {
        *Max::get().player_state() = 0;
      }

      if (fg && ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_select_fg"]) &&
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        selectedTile.tile = fg;
        selectedTile.pos = S32Vec2{rx, ry};
        selectedTile.room = *Max::get().player_room();
        selectedTile.map = *Max::get().player_map();
        editorTile.id = fg->id;
        editorTile.param = fg->param;
        editorTile.flags = fg->flags;
      }

      if (fg && ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_select_bg"]) &&
          !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        selectedTile.tile = fg;
        selectedTile.pos = S32Vec2{rx, ry};
        selectedTile.room = *Max::get().player_room();
        selectedTile.map = *Max::get().player_map();
        editorTile.id = bg->id;
        editorTile.param = bg->param;
        editorTile.flags = bg->flags;
      }

      if (fg && ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_edit_fg"]) &&
          !io.WantCaptureMouse) {
        fg->id = editorTile.id;
        fg->param = editorTile.param;
        fg->flags = editorTile.flags;
      }

      if (bg && ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_edit_bg"]) &&
          !io.WantCaptureMouse) {
        bg->id = editorTile.id;
        bg->param = editorTile.param;
        bg->flags = editorTile.flags;
      }

      if (ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_destroy"]) &&
          !io.WantCaptureMouse) {
        auto broken = true;
        int bit = Max::get().player_room()->y * 20 * 40 * 22 + ry * 20 * 40 +
                  Max::get().player_room()->x * 40 + rx;
        Max::get().map_bits(2)->set(bit, broken);
      }

      if (ImGui::IsKeyChordDown((ImGuiKey)keys["mouse_fix"]) &&
          !io.WantCaptureMouse) {
        auto broken = false;
        int bit = Max::get().player_room()->y * 20 * 40 * 22 + ry * 20 * 40 +
                  Max::get().player_room()->x * 40 + rx;
        Max::get().map_bits(2)->set(bit, broken);
      }
    }

    if (options["ui_coords"].value && inbound &&
        ImGui::GetMouseCursor() != ImGuiMouseCursor_None &&
        !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
      ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
          ->AddRect(TileToScreen(ImVec2(rx, ry)),
                    TileToScreen(ImVec2(rx + 1, ry + 1)), 0xddffffff, 0, 0,
                    3.f);
      int wx = 320 * Max::get().player_room()->x + x;
      int wy = 180 * Max::get().player_room()->y + y;
      int mx = 40 * Max::get().player_room()->x + rx;
      int my = 22 * Max::get().player_room()->y + ry;
      std::string coord = fmt::format(
          "Screen: {},{}\n  Tile: {},{}\n World: {},{}\n   Map: {},{}", x, y,
          rx, ry, wx, wy, mx, my);
      coord += fmt::format("\n Flags: 0x{:X}",
                           Max::get().get_room_tile_flags(rx, ry, 0xffff));
      if (fg && bg)
        coord += fmt::format("\n    ID: {},{}", fg->id, bg->id);
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

  auto ui_scaling =
      options["ui_scaling"].value; // cache value in case it gets changed
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

  if (key_to_change != "")
    KeyCapture();
  else
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
  writeData << "map_scale = " << std::dec << mapScale << " # float, 1 - 5"
            << std::endl;

  writeData << "\n[ui_keys] # hex ImGuiKeyChord\n";
  for (const auto &[name, key] : keys) {
    writeData << name << " = 0x" << std::hex << key << std::endl;
  }

  writeData << "\n[custom_keys] # hex keycode, e.g. 0x20 for space";
  writeData << "\nup = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::UP];
  writeData << "\ndown = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::DOWN];
  writeData << "\nleft = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::LEFT];
  writeData << "\nright = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::RIGHT];
  writeData << "\njump = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::JUMP];
  writeData << "\naction = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::ACTION];
  writeData << "\nitem = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::ITEM];
  writeData << "\ninventory = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::INVENTORY];
  writeData << "\nmap = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::MAP];
  writeData << "\nlb = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::LB];
  writeData << "\nrb = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::RB];
  writeData << "\npause = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::PAUSE];
  writeData << "\nhud = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::HUD];
  writeData << "\ncring = 0x" << std::hex
            << (int)Max::get().keymap[GAME_INPUT::CRING];

  writeData << "\n\n";
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
  mapScale = toml::find_or<float>(opts, "map_scale", 1.f);

  toml::value custom_keys;
  try {
    custom_keys = toml::find(data, "custom_keys");
  } catch (std::exception &) {
    SaveINI();
    return;
  }

  Max::get().keymap[GAME_INPUT::UP] =
      toml::find_or<uint8_t>(custom_keys, "up", 0);
  Max::get().keymap[GAME_INPUT::DOWN] =
      toml::find_or<uint8_t>(custom_keys, "down", 0);
  Max::get().keymap[GAME_INPUT::LEFT] =
      toml::find_or<uint8_t>(custom_keys, "left", 0);
  Max::get().keymap[GAME_INPUT::RIGHT] =
      toml::find_or<uint8_t>(custom_keys, "right", 0);
  Max::get().keymap[GAME_INPUT::JUMP] =
      toml::find_or<uint8_t>(custom_keys, "jump", 0);
  Max::get().keymap[GAME_INPUT::ACTION] =
      toml::find_or<uint8_t>(custom_keys, "action", 0);
  Max::get().keymap[GAME_INPUT::ITEM] =
      toml::find_or<uint8_t>(custom_keys, "item", 0);
  Max::get().keymap[GAME_INPUT::INVENTORY] =
      toml::find_or<uint8_t>(custom_keys, "inventory", 0);
  Max::get().keymap[GAME_INPUT::MAP] =
      toml::find_or<uint8_t>(custom_keys, "map", 0);
  Max::get().keymap[GAME_INPUT::LB] =
      toml::find_or<uint8_t>(custom_keys, "lb", 0);
  Max::get().keymap[GAME_INPUT::RB] =
      toml::find_or<uint8_t>(custom_keys, "rb", 0);
  Max::get().keymap[GAME_INPUT::PAUSE] =
      toml::find_or<uint8_t>(custom_keys, "pause", 0);
  Max::get().keymap[GAME_INPUT::HUD] =
      toml::find_or<uint8_t>(custom_keys, "hud", 0);
  Max::get().keymap[GAME_INPUT::CRING] =
      toml::find_or<uint8_t>(custom_keys, "cring", 0);

  toml::value ui_keys;
  try {
    ui_keys = toml::find(data, "ui_keys");
  } catch (std::exception &) {
    SaveINI();
    return;
  }

  for (const auto &[name, key] : default_keys) {
    keys[name] = (ImGuiKeyChord)toml::find_or<int>(ui_keys, name, (int)key);
  }

  UpdateOptions();

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
