#pragma once
#include <map>
#include <optional>
#include <string>
#include <unordered_map>

struct Setting {
    bool value;
    std::string name;
    std::string desc;
    std::string key;
};

class Settings {
  public:
    std::map<std::string, Setting> options;

    static const std::map<std::string, int> default_keys;
    std::map<std::string, int> keys;

    int windowScale = 4;
    float mapScale = 1.0f;
    int forcedPalette = 26; // not saved to ini

    void SaveINI() const;
    void LoadINI();

    bool CheatsEnabled() {
        return options["cheat_active"].value && (options["ui_visible"].value || !options["ui_ignore_cheats"].value);
    }

    std::optional<uint8_t> forced_palette() {
        if(options["cheat_palette"].value && CheatsEnabled()) {
            return forcedPalette;
        }
        return std::nullopt;
    }
};

inline Settings settings;
