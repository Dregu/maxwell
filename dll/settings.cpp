#include "settings.h"
#include "max.h"

#include <fstream>
#include <imgui.h>
#include <toml.hpp>

static std::map<std::string, Setting> default_options {
    {"cheat_active", {true, "Enable cheats", "Master switch to disable all cheats,\nignoring their current state."}},
    {"cheat_damage", {false, "No damage", "Disables taking any damage, but\nkeeps knockback, drowning etc.", "toggle_damage"}},
    {"cheat_godmode", {false, "God mode", "Disables taking any damage or knockback,\nsimply walk through most threats.", "toggle_godmode"}},
    {"cheat_noclip", {false, "Noclip", "Do the cring thing without the cring.", "toggle_noclip"}},
    {"cheat_darkness", {false, "Disable darkness", "Removes the fog of war effect, see through walls.", "toggle_darkness"}},
    {"cheat_lights", {false, "Disable lamps", "Removes the lighting effect of hanging lights.", "toggle_lights"}},
    {"cheat_palette", {false, "Force lighting", "Forces a specific room color palette/lighting.\n(a very bright one by default)", "toggle_palette"}},
    {"cheat_gameboy", {false, "Gameboy mode", "Party like it's 1989! Enable the B&W death screen shader all the time.", "toggle_gameboy"}},
    {"cheat_groundhog", {false, "Disable Groundhog Day check", "Patches out the Groundhog Day checks,\nso you can always get up to 4 "
                                                               "extra hearts."}},
    {"cheat_hud", {false, "Hide in-game HUD", "Removes all HUD elements, including the menus.", "toggle_hud"}},
    {"cheat_player", {false, "Hide player character", "Doesn't render the player character.", "toggle_player"}},
    {"cheat_clouds", {false, "Disable clouds", "Removes the cloudy foreground effects.", "toggle_clouds"}},
    {"cheat_credits", {false, "Skip credits", "Skips credits and drops the key immediately."}},
    {"cheat_igt", {false, "Use in-game time", "Replaces total game time with\nIGT that doesn't include paused time."}},
    {"cheat_water", {false, "Hide water", "Makes water see-through but still functional."}},
    {"cheat_stats", {false, "Infinite consumables", "Keeps your consumables stacked, including health.", "toggle_stats"}},

    {"input_block", {true, "Block game input on UI input", "Blocks keyboard input from game\nwhen typing or menus are open."}},
    {"input_custom", {false, "Use custom keyboard bindings", "Allows you to rebind all in-game\nkeys and disables default keys."}},
    {"input_mouse", {true, "Mouse controls", "Left click to place tile, middle click to pick tile,\nright click to teleport!", "toggle_mouse"}},

    {"map_auto", {false, "Constantly update minimap", "This can be choppy if map window is left open.\nDefault is to update map when it's opened."}},
    {"map_holes", {false, "Show destroyed tiles on minimap", "Shows what is already destroyed in pink."}},
    {"map_small", {true, "Hide minimap margins", "There's nothing there really except some glitches."}},
    {"map_show", {false, "Show unseen minimap tiles in UI", "Shows what you haven't seen yet, but darker."}},
    {"map_reveal", {false, "Reveal unseen minimap tiles in game", "Marks all tiles seen on the actual in-game map."}},
    {"map_areas", {false, "Show all map borders on minimap", "Draws all special map borders instead of only selected layer."}},
    {"map_wheel", {false, "Show wheel on minimap", "For wheel warping."}},
    {"map_uv_bunny", {true, "Show UV bunny on minimap", "The UV bunny follows a path around the common areas next to the hub\nroom, but turns around if it's about to run into your UV light."}},
    {"map_kangaroo", {true, "Show kangaroo on minimap", "Marks the next kangaroo room with a K."}},

    {"ui_coords", {true, "Show coordinate tooltip", "Show screen and tile coordinates under mouse cursor."}},
    {"ui_grid", {false, "Show tile grid", "Draws a grid of 40 by 22.5 over the screen."}},
    {"ui_tooltips", {true, "Show helpful tooltips", "These are really helpful, why are you disabling them!?"}},
    {"ui_visible", {true, "Show UI", "Hide all MAXWELL windows\nand disable UI input.", "toggle_ui"}},
    {"ui_ignore", {true, "Ignore UI input when hidden", "Ignore mouse and keyboard commands\nwhen UI is hidden with F10."}},
    {"ui_ignore_cheats", {false, "Disable cheats when hidden", "Disables all cheats temporarily\nwhen UI is hidden with F10."}},
    {"ui_viewports", {false, "Multi-viewports", "Allow dragging UI windows outside the game window."}},
    {"ui_scaling", {true, "DPI scaling", "Apply Windows UI scaling.\nMight require a restart to properly resize everything."}},
    {"ui_show_datetime", {true, "Show current time in UI"}},
    {"ui_show_cheats", {true, "Show enabled cheats in UI"}},
    {"ui_debug", {false, "Show debug information"}},
};

const std::map<std::string, ImGuiKeyChord> Settings::default_keys {
    {"escape", ImGuiKey_Escape},
    {"tool_player", ImGuiKey_F1},
    {"tool_map", ImGuiKey_F2},
    {"tool_tools", ImGuiKey_F3},
    {"tool_level", ImGuiKey_F4},
    {"tool_mods", ImGuiKey_F5},
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
    {"reload_mods", ImGuiMod_Ctrl | ImGuiKey_R},
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

void Settings::SaveINI() const {
  // CreateDirectory(L"MAXWELL\\Mods", NULL);
    std::string file = "MAXWELL\\MAXWELL.ini";
    std::ofstream writeData(file);
    writeData << "# MAXWELL options" << std::endl;

    writeData << "\n[options] # 0 or 1 unless stated otherwise\n";
    for(const auto& [name, opt] : options) {
        writeData << name << " = " << std::dec << opt.value << std::endl;
    }
    writeData << "scale_window = " << std::fixed << std::setprecision(2) << windowScale << " # int, 1 - 10" << std::endl;
    writeData << "scale_map = " << std::fixed << std::setprecision(2) << mapScale << " # float, 1 - 5" << std::endl;

    writeData << "\n[ui_keys] # hex ImGuiKeyChord\n";
    for(const auto& [name, key] : keys) {
        writeData << name << " = 0x" << std::hex << key << std::endl;
    }

    auto& max = Max::get();

    writeData << "\n[custom_keys] # hex keycode, e.g. 0x20 for space";
    writeData << "\nup = 0x" << std::hex << (int)max.keymap[GAME_INPUT::UP];
    writeData << "\ndown = 0x" << std::hex << (int)max.keymap[GAME_INPUT::DOWN];
    writeData << "\nleft = 0x" << std::hex << (int)max.keymap[GAME_INPUT::LEFT];
    writeData << "\nright = 0x" << std::hex << (int)max.keymap[GAME_INPUT::RIGHT];
    writeData << "\njump = 0x" << std::hex << (int)max.keymap[GAME_INPUT::JUMP];
    writeData << "\naction = 0x" << std::hex << (int)max.keymap[GAME_INPUT::ACTION];
    writeData << "\nitem = 0x" << std::hex << (int)max.keymap[GAME_INPUT::ITEM];
    writeData << "\ninventory = 0x" << std::hex << (int)max.keymap[GAME_INPUT::INVENTORY];
    writeData << "\nmap = 0x" << std::hex << (int)max.keymap[GAME_INPUT::MAP];
    writeData << "\nlb = 0x" << std::hex << (int)max.keymap[GAME_INPUT::LB];
    writeData << "\nrb = 0x" << std::hex << (int)max.keymap[GAME_INPUT::RB];
    writeData << "\npause = 0x" << std::hex << (int)max.keymap[GAME_INPUT::PAUSE];
    writeData << "\nhud = 0x" << std::hex << (int)max.keymap[GAME_INPUT::HUD];
    writeData << "\ncring = 0x" << std::hex << (int)max.keymap[GAME_INPUT::CRING];

    writeData << "\n[mods]\n";
    for(auto& [name, mod] : max.mods) {
        writeData << '"' << name << "\" = " << (mod.enabled ? "true" : "false") << "\n";
    }

    writeData << "\n\n";
    writeData.close();
}

void Settings::LoadINI() {
    std::string file = "MAXWELL\\MAXWELL.ini";
    if(!std::filesystem::exists(file)) {
        keys = default_keys;
        options = default_options;
        return;
    }

    toml::value data;
    toml::value opts;
    toml::value custom_keys;
    toml::value ui_keys;
    toml::value mods;

    try {
        data = toml::parse(file);
        opts = toml::find(data, "options");
        custom_keys = toml::find(data, "custom_keys");
        ui_keys = toml::find(data, "ui_keys");
        mods = toml::find(data, "mods");
    } catch(std::exception& e) {
        // SaveINI();
        return;
    }

    options = default_options;
    for(auto& [name, opt] : options) {
        opt.value = (bool)toml::find_or<int>(opts, name, (int)opt.value);
    }
    windowScale = toml::find_or<int>(opts, "scale_window", 4);
    mapScale = toml::find_or<float>(opts, "scale_map", 1.f);

    auto& max = Max::get();

    max.keymap[GAME_INPUT::UP] = toml::find_or<uint8_t>(custom_keys, "up", 0);
    max.keymap[GAME_INPUT::DOWN] = toml::find_or<uint8_t>(custom_keys, "down", 0);
    max.keymap[GAME_INPUT::LEFT] = toml::find_or<uint8_t>(custom_keys, "left", 0);
    max.keymap[GAME_INPUT::RIGHT] = toml::find_or<uint8_t>(custom_keys, "right", 0);
    max.keymap[GAME_INPUT::JUMP] = toml::find_or<uint8_t>(custom_keys, "jump", 0);
    max.keymap[GAME_INPUT::ACTION] = toml::find_or<uint8_t>(custom_keys, "action", 0);
    max.keymap[GAME_INPUT::ITEM] = toml::find_or<uint8_t>(custom_keys, "item", 0);
    max.keymap[GAME_INPUT::INVENTORY] = toml::find_or<uint8_t>(custom_keys, "inventory", 0);
    max.keymap[GAME_INPUT::MAP] = toml::find_or<uint8_t>(custom_keys, "map", 0);
    max.keymap[GAME_INPUT::LB] = toml::find_or<uint8_t>(custom_keys, "lb", 0);
    max.keymap[GAME_INPUT::RB] = toml::find_or<uint8_t>(custom_keys, "rb", 0);
    max.keymap[GAME_INPUT::PAUSE] = toml::find_or<uint8_t>(custom_keys, "pause", 0);
    max.keymap[GAME_INPUT::HUD] = toml::find_or<uint8_t>(custom_keys, "hud", 0);
    max.keymap[GAME_INPUT::CRING] = toml::find_or<uint8_t>(custom_keys, "cring", 0);

    for(const auto& [name, key] : default_keys) {
        keys[name] = (ImGuiKeyChord)toml::find_or<int>(ui_keys, name, (int)key);
    }

    if(mods.is_table()) {
        for(auto& [key, val] : mods.as_table()) {
            max.mods[key] = {val.as_boolean(), "MAXWELL/mods/" + key};
        }
    }

  // UpdateOptions();

  /*if (options["ui_viewports"].value) {
      ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    } else {
      ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    }*/
}
