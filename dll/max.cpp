#define UNICODE

#include "max.h"

#include "shlwapi.h"
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "detours.h"
#include "ghidra_byte_string.h"
#include "logger.h"
#include "memory.h"
#include "settings.h"

namespace fs = std::filesystem;

const std::array<uint8_t, 16> encryption_keys[3] = {
    {'G', 'o', 'o', 'd', 'L', 'U', 'c', 'K', 'M', 'y', 'F', 'r', 'i', 'E', 'n', 'd'},
    {0xC9, 0x9B, 0x64, 0x96, 0x5C, 0xCE, 0x04, 0xF0, 0xF5, 0xCB, 0x54, 0xCA, 0xC9, 0xAB, 0x62, 0xC6},
    {0x11, 0x14, 0x18, 0x14, 0x88, 0x82, 0x42, 0x82, 0x28, 0x24, 0x88, 0x82, 0x11, 0x18, 0x44, 0x11}
};

using UpdateState = void(void *a, void *b, void *c, void *d);
UpdateState *g_update_state_trampoline{nullptr};
void HookUpdateState(void *a, void *b, void *c, void *d) {
  if (Max::get().pause()->type == 0) {
    if (Max::get().set_pause.has_value()) {
      if (Max::get().set_pause.value()) {
        Max::get().paused = true;
      } else {
        Max::get().pause()->paused = false;
        Max::get().paused = std::nullopt;
      }
      Max::get().set_pause = std::nullopt;
    }
    if (Max::get().paused.has_value()) {
      Max::get().pause()->paused = Max::get().paused.value();
    }
    if (Max::get().skip && Max::get().paused.value_or(false)) {
      Max::get().pause()->paused = false;
      Max::get().skip = false;
    }
  }
  if (!Max::get().inputs.empty()) {
    Max::get().input = (PLAYER_INPUT)Max::get().inputs.front();
    Max::get().inputs.pop_front();
  }
  g_update_state_trampoline(a, b, c, d);
  if(settings.options["cheat_igt"].value)
    *(Max::get().timer() + 1) = *Max::get().timer();
}

using Void = void();
Void *g_update_input_trampoline{nullptr};
void HookUpdateInput() {
  if (Max::get().pause()->type == 0 && Max::get().paused.value_or(false) &&
      !Max::get().skip)
    return;
  g_update_input_trampoline();
}

using Render = void(void *a, void *b);
Render *g_render_trampoline{nullptr};
void HookRender(void *a, void *b) {
  g_render_trampoline(a, b);
  for (auto f : Max::get().render_queue)
    f();
  Max::get().render_queue.clear();
}

using GetInput = uint32_t(uint16_t a);
GetInput *g_get_input_trampoline{nullptr};
uint32_t HookGetInput(uint16_t a) {
  auto ret = g_get_input_trampoline(a);
  // if (ret > 0)
  //   DEBUG("GetInput: {:x} {:x}", a, ret);

  auto i = Max::get().input;

  if (i == PLAYER_INPUT::SKIP)
    return ret;

  if (i == PLAYER_INPUT::NONE)
    return 0;

  if (a & i) {
    if (a >= PLAYER_INPUT::UP && a <= PLAYER_INPUT::RIGHT)
      ret = 1;
    else if (a == PLAYER_INPUT::LB)
      ret = 0x401;
    else if (a == PLAYER_INPUT::RB)
      ret = 0x801;
    else if (a == PLAYER_INPUT::JUMP)
      ret = 0x4001;
    else if (a == PLAYER_INPUT::ACTION)
      ret = 0x2001;
    else if (a == PLAYER_INPUT::ITEM)
      ret = 0x8001;
    else if (a == PLAYER_INPUT::INVENTORY)
      ret = 0x1001;
    else if (a == PLAYER_INPUT::MAP)
      ret = 0x100001;
  }
  return ret;
}

using GetRoomParams = RoomParams(void *a, uint16_t b);
GetRoomParams *g_room_params_trampoline{nullptr};
RoomParams HookGetRoomParams(void *a, uint16_t b) {
  auto ret = g_room_params_trampoline(a, b);
  auto pal = settings.forced_palette();
  if (pal.has_value() && pal.value() >= 0 && pal.value() <= 31)
    ret.palette = pal.value();
  return ret;
}

using GetRoomWater = RoomParams(void *a, uint16_t b);
GetRoomWater *g_room_water_trampoline{nullptr};
RoomParams HookGetRoomWater(void *a, uint16_t b) {
  auto ret = g_room_water_trampoline(a, b);
  auto wat = Max::get().force_water;
  if (wat.has_value())
    ret.palette = wat.value();
  return ret;
}

using SetupGame = void(void *);
SetupGame *g_setup_game_trampoline{nullptr};
void HookSetupGame(void *a) {
  Max::get().reload_mods(true);
  g_setup_game_trampoline(a);
}

GAME_INPUT KeyToInput(uint8_t vk) {
  switch (vk) {
  case 'W':
  case VK_UP:
  case VK_NUMPAD8:
    return GAME_INPUT::UP;
  case 'S':
  case VK_DOWN:
  case VK_NUMPAD2:
    return GAME_INPUT::DOWN;
  case 'A':
  case VK_LEFT:
  case VK_NUMPAD4:
    return GAME_INPUT::LEFT;
  case 'D':
  case VK_RIGHT:
  case VK_NUMPAD6:
    return GAME_INPUT::RIGHT;
  case 'V':
  case 'M':
    return GAME_INPUT::MAP;
  case '1':
    return GAME_INPUT::LB;
  case '3':
    return GAME_INPUT::RB;
  case VK_SPACE:
  case VK_NUMPAD0:
    return GAME_INPUT::JUMP;
  case 'Q':
  case 'Z':
  case VK_RETURN:
  case VK_BACK:
    return GAME_INPUT::ACTION;
  case 'E':
  case 'X':
  case VK_NUMPAD5:
    return GAME_INPUT::ITEM;
  case '2':
  case 'R':
  case 'I':
  case 'C':
    return GAME_INPUT::INVENTORY;
  case VK_ESCAPE:
  case 'P':
    return GAME_INPUT::PAUSE;
  case 'H':
    return GAME_INPUT::HUD;
  case 'F':
    return GAME_INPUT::CRING;
  default:
    return GAME_INPUT::NONE;
  }
  return GAME_INPUT::NONE;
}

uint8_t GetMappedKey(uint8_t vk) {
  auto i = KeyToInput(vk);
  if (i != GAME_INPUT::NONE && Max::get().keymap.contains(i))
    return Max::get().keymap[i];
  return 0;
}

GAME_INPUT IconToInput(BUTTON_ICON c) {
  switch (c) {
  case BUTTON_ICON::JUMP:
    return GAME_INPUT::JUMP;
  case BUTTON_ICON::ACTION:
    return GAME_INPUT::ACTION;
  case BUTTON_ICON::ITEM:
    return GAME_INPUT::ITEM;
  case BUTTON_ICON::INVENTORY:
    return GAME_INPUT::INVENTORY;
  case BUTTON_ICON::LB:
    return GAME_INPUT::LB;
  case BUTTON_ICON::RB:
    return GAME_INPUT::RB;
  default:
    return GAME_INPUT::NONE;
  }
  return GAME_INPUT::NONE;
}

using KeyPressed = bool(uint8_t);
KeyPressed *g_key_pressed_trampoline{nullptr};
bool HookKeyPressed(uint8_t vk) {
  if (!settings.options["input_custom"].value || vk == VK_LBUTTON || (vk & VK_NUMPAD0) == VK_NUMPAD0)
    return g_key_pressed_trampoline(vk);
  auto mk = GetMappedKey(vk);
  if (mk)
    return g_key_pressed_trampoline(mk);
  return false;
}

using KeyDown = uint8_t(uint8_t);
KeyDown *g_key_down_trampoline{nullptr};
uint8_t HookKeyDown(uint8_t vk) {
  if (!settings.options["input_custom"].value || vk == VK_LBUTTON ||
      (vk & VK_NUMPAD0) == VK_NUMPAD0)
    return g_key_down_trampoline(vk);
  auto mk = GetMappedKey(vk);
  if (mk)
    return g_key_down_trampoline(mk);
  return 0;
}

using DrawButton = void(int x, int y, BUTTON_ICON c);
DrawButton *g_draw_action_button_trampoline{nullptr};
void HookDrawActionButton(int x, int y, BUTTON_ICON c) {
  if (Max::get().keymap.empty() || !settings.options["input_custom"].value ||
      *(Max::get().options() + 0x10) != 3) {
    g_draw_action_button_trampoline(x, y, c);
    return;
  }
  g_draw_action_button_trampoline(x, y, BUTTON_ICON::ACTION);
  Max::get().draw_text_small(x - 7, y + 2, L"Z", 0xff000000);

  auto i = IconToInput(c);
  if (i != GAME_INPUT::NONE && Max::get().keymap.contains(i)) {
    if (Max::get().keymap[i] == VK_SPACE) {
      g_draw_action_button_trampoline(x, y, BUTTON_ICON::JUMP);
    } else if (Max::get().keymap[i] == 'X') {
      g_draw_action_button_trampoline(x, y, BUTTON_ICON::ITEM);
    } else if (Max::get().keymap[i] >= '0' && Max::get().keymap[i] <= 'Z') {
      const std::wstring ch{(wchar_t)Max::get().keymap[i]};
      Max::get().draw_text_small(x - 7, y + 2, ch.data());
    } else {
      Max::get().draw_text_small(x - 7, y + 2, L"?");
    }
  }
}

DrawButton *g_draw_button_trampoline{nullptr};
void HookDrawButton(int x, int y, BUTTON_ICON c) {
  if (Max::get().keymap.empty() || !settings.options["input_custom"].value ||
      *(Max::get().options() + 0x10) != 3) {
    g_draw_button_trampoline(x, y, c);
    return;
  }
  auto i = IconToInput(c);
  if (i != GAME_INPUT::NONE && Max::get().keymap.contains(i)) {
    if (Max::get().keymap[i] == VK_SPACE) {
      g_draw_button_trampoline(x, y, BUTTON_ICON::JUMP);
    } else if (Max::get().keymap[i] == 'X') {
      g_draw_button_trampoline(x, y, BUTTON_ICON::ITEM);
    } else if (Max::get().keymap[i] >= '0' && Max::get().keymap[i] <= 'Z') {
      g_draw_button_trampoline(x, y, BUTTON_ICON::ACTION);
      Max::get().draw_text_small(x + 2, y + 2, L"Z", 0xff000000);
      const std::wstring ch{(wchar_t)Max::get().keymap[i]};
      Max::get().draw_text_small(x + 2, y + 2, ch.data());
    } else {
      g_draw_button_trampoline(x, y, BUTTON_ICON::ACTION);
      Max::get().draw_text_small(x + 2, y + 2, L"Z", 0xff000000);
      Max::get().draw_text_small(x + 2, y + 2, L"?");
    }
  }
}

inline bool &get_is_init() {
  static bool is_init{false};
  return is_init;
}

Max &Max::get() {
  static Max MAX;

  if (!get_is_init()) {
    preload_addresses();

    if (auto off = get_address("check")) {
      write_mem_recoverable("check", off, "E9 24 01 00 00 90"_gh, true);
    }

    if (auto off = get_address("patch")) {
      write_mem_recoverable("patch", off, "30"_gh, true);
    }

    if (g_update_state_trampoline =
            (UpdateState *)get_address("update_state")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_update_state_trampoline, HookUpdateState);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking UpdateState: {}\n", error);
      }
    }

    if (g_update_input_trampoline = (Void *)get_address("update_input")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_update_input_trampoline, HookUpdateInput);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking UpdateInput: {}\n", error);
      }
    }

    if (g_get_input_trampoline = (GetInput *)get_address("get_input")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_get_input_trampoline, HookGetInput);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking GetInput: {}\n", error);
      }
    }

    if (g_render_trampoline = (Render *)get_address("render")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_render_trampoline, HookRender);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking Render: {}\n", error);
      }
    }

    if (g_room_params_trampoline =
            (GetRoomParams *)get_address("get_room_params")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_room_params_trampoline, HookGetRoomParams);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking GetRoomParams: {}\n", error);
      }
    }

    /* TODO if (g_room_water_trampoline =
            (GetRoomWater *)get_address("get_room_water")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_room_water_trampoline, HookGetRoomWater);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking GetRoomWater: {}\n", error);
      }
    }*/

    if (g_setup_game_trampoline = (SetupGame *)get_address("setup_game")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_setup_game_trampoline, HookSetupGame);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking SetupGame: {}\n", error);
      }
    }

    if (g_key_pressed_trampoline = (KeyPressed *)get_address("key_pressed")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_key_pressed_trampoline, HookKeyPressed);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking KeyPressed: {}\n", error);
      }
    }

    if (g_key_down_trampoline = (KeyDown *)get_address("key_down")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_key_down_trampoline, HookKeyDown);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking KeyDown: {}\n", error);
      }
    }

    if (g_draw_button_trampoline = (DrawButton *)get_address("draw_button")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_draw_button_trampoline, HookDrawButton);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking DrawButton: {}\n", error);
      }
    }

    if (g_draw_action_button_trampoline =
            (DrawButton *)get_address("draw_action_button")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_draw_action_button_trampoline,
                   HookDrawActionButton);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking DrawActionButton: {}\n", error);
      }
    }

    get_is_init() = true;
  }
  return MAX;
}

void Max::unhook() {
  if (g_update_state_trampoline) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((void **)&g_update_state_trampoline, HookUpdateState);
    const LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
      DEBUG("Failed unhooking UpdateState: {}\n", error);
    }
    g_update_state_trampoline = nullptr;
  }

  if (g_update_input_trampoline) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((void **)&g_update_input_trampoline, HookUpdateInput);
    const LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
      DEBUG("Failed unhooking UpdateInput: {}\n", error);
    }
    g_update_input_trampoline = nullptr;
  }

  if (g_get_input_trampoline) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((void **)&g_get_input_trampoline, HookGetInput);
    const LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
      DEBUG("Failed unhooking GetInput: {}\n", error);
    }
    g_get_input_trampoline = nullptr;
  }

  if (g_render_trampoline) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((void **)&g_render_trampoline, HookRender);
    const LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
      DEBUG("Failed unhooking Render: {}\n", error);
    }
    g_render_trampoline = nullptr;
  }

  if (g_room_params_trampoline) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach((void **)&g_room_params_trampoline, HookGetRoomParams);
    const LONG error = DetourTransactionCommit();
    if (error != NO_ERROR) {
      DEBUG("Failed unhooking GetRoomParams: {}\n", error);
    }
    g_room_params_trampoline = nullptr;
  }
}

State Max::state() {
  using GetStateFunc = State();
  static GetStateFunc *get_state = (GetStateFunc *)get_address("get_state_func");
  return get_state();
}

// application_state.render_data.minimap_data
Minimap Max::minimap() { return *(size_t *)get_address("slots") + 0x2490b8; }

SaveData *Max::save() {
  return (SaveData *)(*(size_t *)get_address("slots") + 0x400);
}

// application_state.current_save
uint8_t *Max::slot_number() {
  return (uint8_t *)(*(size_t *)get_address("slots") + 0x40c);
}

// application_state.save_data[application_state.current_save]
Slot Max::slot() {
  return *(size_t *)get_address("slots") + 0x418 + SLOT_SIZE * (*slot_number());
}

// application_state.game_state.player_data
Player Max::player() {
  return (Player)(*(size_t *)get_address("slots") + 0x93670);
}

// application_state.game_state.player_data.room_coordinates
S32Vec2 *Max::player_room() { return (S32Vec2 *)(player() + 0x20); }

// application_state.game_state.player_data.position
FVec2 *Max::player_position() { return (FVec2 *)(player()); }

// application_state.game_state.player_data.velocity
FVec2 *Max::player_velocity() { return (FVec2 *)(player() + 0x8); }

FVec2 *Max::player_wheel() {
  return (FVec2 *)(*(size_t *)get_address("slots") + 0x9b0c0);
}

FVec2 *Max::uv_bunny() {
  return (FVec2 *)(*(size_t *)get_address("slots") + 0x754a8 + 0x30ec8);
}

int *Max::player_map() {
  return (int *)(*(size_t *)get_address("layer_base") +
                 *(uint32_t *)get_address("layer_offset"));
}

S32Vec2 *Max::respawn_room() { return (S32Vec2 *)(player() + 0x98); }

S32Vec2 *Max::respawn_position() { return (S32Vec2 *)(player() + 0xa0); }

S32Vec2 *Max::warp_room() { return (S32Vec2 *)(player() + 0x34); }

S32Vec2 *Max::warp_position() { return (S32Vec2 *)(player() + 0x3c); }

int *Max::warp_map() { return (int *)(player() + 0x44); }

uint8_t *Max::player_state() { return (uint8_t *)(player() + 0x5d); }

uint8_t *Max::player_flute() { return (uint8_t *)(player() + 0x8955); }

Directions *Max::player_directions() {
  return (Directions *)(player() + 0x891c);
}

// save_data.total_hearts
int8_t *Max::player_hp() { return (int8_t *)(slot() + 0x1b4); }

// save_data.save_position
S32Vec2 *Max::spawn_room() { return (S32Vec2 *)(slot() + 0x1d4); }

// save_data.opened_equipment
uint16_t *Max::equipment() { return (uint16_t *)(slot() + 0x1dc); }

// save_data.owned_items
uint8_t *Max::items() { return (uint8_t *)(slot() + 0x1de); }

// save_data.upgrades
uint32_t *Max::upgrades() { return (uint32_t *)(slot() + 0x1ec); }

// save_data.key_count
uint8_t *Max::keys() { return (uint8_t *)(slot() + 0x1b1); }

// save_data.selected_item
uint8_t *Max::item() { return (uint8_t *)(slot() + 0x1ea); }

// application_state.?
uint8_t *Max::options() {
  return (uint8_t *)(*(size_t *)get_address("slots") + 0x75448);
}

// save_data.eggs
uint64_t *Max::eggs() { return (uint64_t *)(slot() + 0x188); }

// save_data.flames
uint8_t *Max::flames() { return (uint8_t *)(slot() + 0x21e); }

// save_data.opened_chests
uint64_t *Max::chests() { return (uint64_t *)(slot() + 0x120); }

// save_data.candle_flags
uint16_t *Max::candles() { return (uint16_t *)(slot() + 0x1e0); }

// save_data.bunnies_collected
uint32_t *Max::bunnies() { return (uint32_t *)(slot() + 0x198); }

// save_data.squirrels_collected
uint16_t *Max::squirrels() { return (uint16_t *)(slot() + 0x19c); }

// save_data.kangaroo_data
Kangaroo *Max::kangaroo() { return (Kangaroo *)(slot() + 0x1f4); }

// save_data.teleports_seen
uint8_t *Max::portals() { return (uint8_t *)(slot() + 0x223); }

// save_data.kangaroo_data.encounters[0].shard_state
uint8_t *Max::shards() { return (uint8_t *)(slot() + 0x1F4 + 0xa); }

// save_data.ui_flags
uint16_t *Max::progress() { return (uint16_t *)(slot() + 0x21c); }

// save_data.BlueManticore_state
uint8_t *Max::manticore() { return (uint8_t *)(slot() + 0x1F0); }

Pause *Max::pause() {
  return (Pause *)((*(size_t *)get_address("slots") + 0x93608));
};

// save_data.frams_in_game
uint32_t *Max::timer() { return (uint32_t *)(slot() + 0x1bc); }

// save_data.steps
uint32_t *Max::steps() { return (uint32_t *)(slot() + 0x108); }

// save_data.mural_cursor_pos
uint8_t *Max::mural_selection() { return (uint8_t *)(slot() + 0x3ea); }

// save_data.revealed_map / pencil_drawing / deleted_tiles
std::bitset<0xce40 * 8> *Max::map_bits(int n) {
  return (std::bitset<0xce40 * 8> *)(slot() + 0x3ec + n * 0xce41);
}

// save_data.mural
std::array<uint8_t, 200> *Max::mural() {
  return (std::array<uint8_t, 200> *)(slot() + 0x26eaf);
}

// globals.maps[m]
Map *Max::map(int m) {
  if (m >= 0 && m <= 4)
    return (Map *)(*(size_t *)get_address("layer_base") + 0x2d0 + m * sizeof(Map));
  return nullptr;
}

// globals.maps[m].rooms[room_lookup[y][x])]
Room *Max::room(int m, int x, int y) {
  auto layer = Max::get().map(m);
  int room_id = layer->room_lookup[y][x];
  if(room_id != -1)
    return &layer->rooms[room_id];
  return nullptr;
}

LightingData *Max::lighting(int id) {
  static std::unique_ptr<std::array<LightingData, 32>> data = nullptr;

  if (!data) { // the game does not copy the lighting data to memory so we have
               // to sneak in our own pointer to be able to edit the data.
    data = std::make_unique<std::array<LightingData, 32>>();

    auto data_addr = (LightingData **)(*(size_t *)get_address("layer_base") + 0x89d470);
    memcpy(data->data(), *data_addr, data->size() * sizeof(LightingData));
    *data_addr = data->data();
  }

  if (id >= 0 && id < 32)
    return &(*data)[id];
  return nullptr;
}

Tile *Max::tile(int m, int rx, int ry, int x, int y, int l) {
  auto room = Max::room(m, rx, ry);
  if (room)
    return &room->tiles[l][y][x];
  return nullptr;
}

bool Max::import_map(const std::string& file, int m) {
  auto layer = Max::get().map(m);
  if (!layer)
    return false;

  MapHeader header;
  std::ifstream f {file, std::ios::binary};
  if (!f.good())
    return false;

  f.read((char*)&header, sizeof(MapHeader));
  if(header.signature1 != 0xF00DCAFE || header.signature2 != 0xF0F0CAFE) {
    return false;
  }

  // DEBUG("Room Count: {}", header.roomCount);
  layer->roomCount = header.roomCount;
  layer->world_wrap_x_start = header.world_wrap_x_start;
  layer->world_wrap_x_end = header.world_wrap_x_end;
  f.read(reinterpret_cast<char *>(&layer->rooms), sizeof(layer->rooms) + sizeof(layer->room_lookup));

  return true;
}

void Max::load_maps_from_asset() {
  static const std::array map_to_asset{300, 157, 193, 52, 222};
  using LoadFunc = void(void* data, Map* map);
  static LoadFunc* load = (LoadFunc*)get_address("load_map_from_data");

  for (size_t i = 0; i < 5; i++) {
    auto map = this->map(i);
    auto asset = get_asset(map_to_asset[i]);
    load(asset->data, map);
  }

  {
    auto lights = lighting(0); // maxwell replaces the lighting pointer so overwrite that with the loaded asset data
    auto asset = get_asset(179);
    std::memcpy(lights, (uint8_t*)asset->data + 0xC, sizeof(LightingData) * 32);
  }

  {
      auto uvs = (uv_data**)((*(size_t*)get_address("layer_base")) + 0x89d068);
      auto asset = get_asset(254);
      *uvs = (uv_data*)((uint8_t*)asset->data + 0xC);
  }

  // doesn't work and idk why
  /*{ // update texture atlas gpu memory
      auto asset = get_asset(255);
      Image img((char*)asset->data, asset->size);

      auto s = img.size();
      for(size_t i = 0; i < s.x * s.y; i++) {
          img.data()[i] = 0;
      }

      // application_state.render_data.mainTextureAtlas
      auto texture = *(size_t*)(*(size_t*)get_address("slots") + 0x248120 + 0x600);

      using UploadFunc = void(void* texture, void* data);
      static auto upload = (UploadFunc*)get_address("upload_texture");
      upload((void*)texture, img.data());
  }*/

  update_room();
}

void Max::update_room() {
    using WarpFunc = void(void* room, S32Vec2 room_coordinates, void* save_data);
    static auto warp = (WarpFunc*)get_address("room_changed");
    warp((void*)(*(size_t*)get_address("slots") + 0x754a8 + 0x28), *player_room(), (void*)slot());
}

void Max::save_game() {
  using SaveGameFunc = void();
  static SaveGameFunc *save = (SaveGameFunc *)get_address("save_game");
  return save();
}

AssetInfo *Max::decrypt_asset(uint32_t id, int key) {
  using DecryptFunc = AssetInfo *(uint32_t id, const uint8_t * key);
  static DecryptFunc *decrypt = (DecryptFunc *)get_address("decrypt_asset");
  return decrypt(id, encryption_keys[key].data());
}

AssetInfo *Max::get_asset(uint32_t id) {
  return (AssetInfo *)(get_address("assets") + id * sizeof(AssetInfo));
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static int readInt(const std::string& str) {
    if(!isDigit(str[0]))
        return -1;
    int res = 0;
    for (auto &c : str) {
        if(isDigit(c)) {
            res *= 10;
            res += c - '0';
        } else {
            break;
        }
    }
    return res;
}

static std::vector<uint8_t> readFile(const std::string& path) {
    if(!std::filesystem::exists(path))
        throw std::runtime_error("File not found");

    std::ifstream testFile(path, std::ios::binary);
    testFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    return std::vector<uint8_t>(std::istreambuf_iterator(testFile), std::istreambuf_iterator<char>());
}

static void toLower(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
}

static std::unordered_map<int, std::vector<uint8_t>> asset_cache;
static std::array<AssetInfo, 676> original_assets;

void Max::load_tile_mods() {
  AssetInfo &asset = *get_asset(255);

  Image atlas((char *)asset.data, asset.size);
  DEBUG("Created atlas texture for tile mods");

  auto& uvs = *tile_uvs();

  bool changed = false;
  for(auto& [_, mod] : mods) {
    if(!mod.enabled || !fs::exists(mod.path / "tiles")) continue;

    for (auto &file : fs::directory_iterator(mod.path / "tiles")) {
      auto ext = file.path().extension().string();
      toLower(ext);
      if(!file.is_regular_file() || ext != ".png") continue;
      int id = readInt(file.path().filename().string());
      if(id == -1) continue;

      changed = true;
      Image img(file.path().string());
      auto size = img.size();
      auto pos = uvs[id].pos;
      for (int y1 = 0; y1 < size.y; y1++) {
        for (int x1 = 0; x1 < size.x; x1++) {
          atlas(pos.x + x1, pos.y + y1) = img(x1, y1);
        }
      }
      DEBUG("Loaded tile {} from {}", id, file.path().string());
    }
  }

  if(changed) {
    int len;
    auto png = (uint8_t*)atlas.get_png(&len);;
    asset_cache[255] = std::vector<uint8_t>(png, png + len);
    asset.data = asset_cache[255].data();
    asset.size = asset_cache[255].size();
    asset.type &= AssetType::Normal;
    free(png);
    DEBUG("Saved atlas texture to asset 255");
  }
}

void Max::restore_original() {
  std::memcpy(get_asset(0), &original_assets, sizeof(original_assets));
  load_maps_from_asset();
}

void Max::update_mod_list() {
  fs::create_directories("MAXWELL/mods");
  for(auto& mod : fs::directory_iterator("MAXWELL/mods")) {
    if(!mod.is_directory()) continue;

    auto name = mod.path().filename().string();
    if(!mods.contains(name)) {
      mods[name] = { false, mod.path(), false };
    }
  }

  std::erase_if(mods, [](auto& kv) { return !std::filesystem::exists(kv.second.path); });
}

// on preload only load things into the asset array since the game has not yet populated the other data
void Max::reload_mods(bool preload) {
  if(preload) {
    // pre decrypt all assets so we won't have to worry about it anymore
    decrypt_asset(193, 0); // space
    decrypt_asset(212, 0); // origami instructions
    decrypt_asset(255, 0); // texture atlas
    decrypt_asset(300, 0); // overworld

    decrypt_asset(30, 1);  // space bunny
    decrypt_asset(52, 1);  // bunny temple

    decrypt_asset(222, 2); // time capsule
    decrypt_asset(277, 2); // time capsule texture
    decrypt_asset(377, 2); // time capsule audio

    std::memcpy(&original_assets, get_asset(0), sizeof(original_assets));
  } else {
    std::memcpy(get_asset(0), &original_assets, sizeof(original_assets));
  }

  update_mod_list();

  std::unordered_map<int, std::string> assets_;

  for(auto& [_, mod] : mods) {
    mod.overlap = false;
    if(!mod.enabled || !fs::exists(mod.path / "assets")) continue;

    for (auto &file : fs::directory_iterator(mod.path / "assets")) {
      auto id = readInt(file.path().filename().string());
      if(id == -1) continue;
      if(assets_.contains(id)) {
          mod.overlap = true;
          continue;
      }
      assets_[id] = file.path().string();
    }
  }

  for (auto &[id, file] : assets_) {
    // keep track of allocated data to avoid memory leak
    asset_cache[id] = readFile(file);
    auto asset = get_asset(id);
    asset->data = asset_cache[id].data();
    asset->size = asset_cache[id].size();
  }

  load_tile_mods();

  if(!preload) {
    load_maps_from_asset();
  }
}

using DrawTextFunc = void(int x, int y, const wchar_t *text);
using PushColorFunc = void(uint32_t color);
using PopColorFunc = void();
void Max::draw_text_big(int x, int y, const wchar_t *text) {
  static DrawTextFunc *draw = (DrawTextFunc *)get_address("draw_text_big");
  draw(x, y, text);
}
void Max::draw_text_small(int x, int y, const wchar_t *text, uint32_t color, uint32_t shader) {
  static DrawTextFunc *draw = (DrawTextFunc *)get_address("draw_text_small");
  static PushColorFunc *push_color = (PushColorFunc *)get_address("draw_push_color");
  static PopColorFunc *pop_color = (PopColorFunc *)get_address("draw_pop_color");
  static PushColorFunc *push_shader = (PushColorFunc *)get_address("draw_push_shader");
  static PopColorFunc *pop_shader = (PopColorFunc *)get_address("draw_pop_shader");
  push_color(color);
  push_shader(shader);
  draw(x, y, text);
  pop_shader();
  pop_color();
}

std::array<uv_data, 1024> *Max::tile_uvs() {
  return (std::array<uv_data, 1024> *)((size_t)get_asset(254)->data + 0xc);
}

uint16_t Max::get_room_tile_flags(int x, int y, uint16_t mask) {
  using GetFunc = uint16_t(int x, int y, uint16_t mask);
  static GetFunc *get = (GetFunc *)get_address("room_tile_flags");
  return get(x, y, mask);
}

void Max::dump_lighting() {
  CreateDirectory(L"MAXWELL\\Dump", NULL);
  CreateDirectory(L"MAXWELL\\Dump\\Assets", NULL);
  std::string file = "MAXWELL\\Dump\\Assets\\179.ambient";
  std::ofstream out(file, std::ios::binary);
  out << "00 0B F0 00 20 00 00 00 00 00 00 00"_gh;
  out.write(reinterpret_cast<char *>(lighting(0)), 32 * sizeof(LightingData));
  out.close();
}

void Max::dump_map(uint8_t m) {
  static const std::array map_to_asset{300, 157, 193, 52, 222};
  auto map = Max::get().map(m);
  auto asset = Max::get().get_asset(map_to_asset[m]);
  CreateDirectory(L"MAXWELL\\Dump", NULL);
  CreateDirectory(L"MAXWELL\\Dump\\Maps", NULL);
  std::string file = fmt::format("MAXWELL\\Dump\\Maps\\{}.map", m);
  std::ofstream out(file, std::ios::binary);
  MapHeader header{0xF00DCAFE,
                   map->roomCount,
                   map->world_wrap_x_start,
                   map->world_wrap_x_end,
                   8,
                   0xF0F0CAFE};
  out.write(reinterpret_cast<char *>(&header), sizeof(MapHeader));
  out.write(reinterpret_cast<char *>(&map->rooms),
            map->roomCount * sizeof(Room));
  out.close();
}

void Max::dump_asset(uint32_t id) {
  auto asset = Max::get().get_asset(id);
  CreateDirectory(L"MAXWELL\\Dump", NULL);
  CreateDirectory(L"MAXWELL\\Dump\\Assets", NULL);
  auto ptr = (uint8_t *)asset->data;
  if (ptr == 0)
    return;
  std::string ext = ".bin";
  if (asset->type == AssetType::Text) {
    ext = ".txt";
  } else if (ptr[0] == 'O' && ptr[1] == 'g' && ptr[2] == 'g' && ptr[3] == 'S') {
    ext = ".ogg";
  } else if (ptr[0] == 0x89 && ptr[1] == 'P' && ptr[2] == 'N' &&
             ptr[3] == 'G') {
    ext = ".png";
  } else if (ptr[0] == 0xFE && ptr[1] == 0xCA && ptr[2] == 0x0D &&
             ptr[3] == 0xF0) {
    ext = ".map";
  } else if (ptr[0] == 'D' && ptr[1] == 'X' && ptr[2] == 'B' && ptr[3] == 'C') {
    ext = ".shader";
  } else if (ptr[0] == 0 && ptr[1] == 0x0B && ptr[2] == 0xB0 && ptr[3] == 0) {
    ext = ".tiles";
  } else if (ptr[0] == 'P' && ptr[1] == 'K' && ptr[2] == 3 && ptr[3] == 4) {
    ext = ".xps";
  } else if (ptr[0] == 0x00 && ptr[1] == 0x0B && ptr[2] == 0xF0 &&
             ptr[3] == 0x00) {
    ext = ".ambient";
  } else if (ptr[0] == 0x1D && ptr[1] == 0xAC) { // ptr[2] = version 1,2,3
    ext = ".sprite";
  } else if (ptr[0] == 'B' && ptr[1] == 'M' && ptr[2] == 'F') {
    ext = ".font";
  }
  auto filename = fmt::format("MAXWELL\\Dump\\Assets\\{}{}", id, ext);
  std::ofstream file(filename, std::ios::binary);
  file.write((char *)asset->data, asset->size);
  file.close();
}
