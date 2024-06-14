#include "max.h"

#include <Windows.h>
#include <array>
#include <filesystem>
#include <fstream>

#include "detours.h"
#include "ghidra_byte_string.h"
#include "logger.h"
#include "memory.h"

namespace fs = std::filesystem;

static std::unordered_map<uint8_t, std::unordered_map<uint16_t, Room *>> rooms;

const std::array<uint8_t, 16> encryption_keys[3] = {
    {'G', 'o', 'o', 'd', 'L', 'U', 'c', 'K', 'M', 'y', 'F', 'r', 'i', 'E', 'n',
     'd'},
    {0xC9, 0x9B, 0x64, 0x96, 0x5C, 0xCE, 0x04, 0xF0, 0xF5, 0xCB, 0x54, 0xCA,
     0xC9, 0xAB, 0x62, 0xC6},
    {0x11, 0x14, 0x18, 0x14, 0x88, 0x82, 0x42, 0x82, 0x28, 0x24, 0x88, 0x82,
     0x11, 0x18, 0x44, 0x11}};

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
  auto pal = Max::get().force_palette;
  if (pal.has_value() && pal.value() >= 0 && pal.value() <= 31)
    ret.palette = pal.value();
  return ret;
}

using GetAsset = AssetInfo(uint32_t id);
GetAsset *g_get_asset_trampoline{nullptr};
AssetInfo HookGetAsset(uint32_t id) {
  auto asset = g_get_asset_trampoline(id);
  Max::get().load_custom_asset(id, asset);
  // DEBUG("GetAsset: {} {}", id, asset.data);
  return asset;
}

using DecryptAsset = AssetInfo *(uint32_t id, uint8_t *key);
DecryptAsset *g_decrypt_asset_trampoline{nullptr};
AssetInfo *HookDecryptAsset(uint32_t id, uint8_t *key) {
  auto *asset = g_decrypt_asset_trampoline(id, key);
  Max::get().load_custom_asset(id, *asset);
  // DEBUG("DecryptAsset: {} {}", id, asset->data);
  return asset;
}

using SetupGame = void(void *);
SetupGame *g_setup_game_trampoline{nullptr};
void HookSetupGame(void *a) {
  g_setup_game_trampoline(a);
  Max::get().load_mods();
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

    if (g_get_asset_trampoline = (GetAsset *)get_address("get_asset")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_get_asset_trampoline, HookGetAsset);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking GetAsset: {}\n", error);
      }
    }

    if (g_decrypt_asset_trampoline =
            (DecryptAsset *)get_address("decrypt_asset")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_decrypt_asset_trampoline, HookDecryptAsset);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking DecryptAsset: {}\n", error);
      }
    }

    if (g_setup_game_trampoline = (SetupGame *)get_address("setup_game")) {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_setup_game_trampoline, HookSetupGame);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking SetupGame: {}\n", error);
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
  static GetStateFunc *get_state =
      (GetStateFunc *)get_address("get_state_func");
  return get_state();
}

Minimap Max::minimap() { return *(size_t *)get_address("slots") + 0x2490b8; }

uint8_t *Max::slot_number() {
  return (uint8_t *)(*(size_t *)get_address("slots") + 0x40c);
}

Slot Max::slot() {
  return *(size_t *)get_address("slots") + SLOT_SIZE * (*slot_number());
}

Player Max::player() {
  return (Player)(*(size_t *)get_address("slots") + 0x93670);
}

Coord *Max::player_room() { return (Coord *)(player() + 0x20); }

fCoord *Max::player_position() { return (fCoord *)(player()); }

fCoord *Max::player_velocity() { return (fCoord *)(player() + 0x8); }

fCoord *Max::player_wheel() {
  return (fCoord *)(*(size_t *)get_address("slots") + 0x9b0c0);
}

int *Max::player_map() {
  return (int *)(*(size_t *)get_address("layer_base") +
                 *(uint32_t *)get_address("layer_offset"));
}

Coord *Max::respawn_room() { return (Coord *)(player() + 0x98); }

Coord *Max::respawn_position() { return (Coord *)(player() + 0xa0); }

Coord *Max::warp_room() { return (Coord *)(player() + 0x34); }

Coord *Max::warp_position() { return (Coord *)(player() + 0x3c); }

int *Max::warp_map() { return (int *)(player() + 0x44); }

uint8_t *Max::player_state() { return (uint8_t *)(player() + 0x5d); }

uint8_t *Max::player_flute() { return (uint8_t *)(player() + 0x8955); }

Directions *Max::player_directions() {
  return (Directions *)(player() + 0x891c);
}

int8_t *Max::player_hp() { return (int8_t *)(slot() + 0x400 + 0x1cc); }

Coord *Max::spawn_room() { return (Coord *)(slot() + 0x400 + 0x1ec); }

uint16_t *Max::equipment() { return (uint16_t *)(slot() + 0x400 + 0x1f4); }

uint8_t *Max::items() { return (uint8_t *)(slot() + 0x400 + 0x1f6); }

uint32_t *Max::upgrades() { return (uint32_t *)(slot() + 0x400 + 0x204); }

uint8_t *Max::keys() { return (uint8_t *)(slot() + 0x400 + 0x1c9); }

uint8_t *Max::item() { return (uint8_t *)(slot() + 0x400 + 0x202); }

uint8_t *Max::options() {
  return (uint8_t *)(*(size_t *)get_address("slots") + 0x400 + 0x75048);
}

Pause *Max::pause() {
  return (Pause *)((*(size_t *)get_address("slots") + 0x93608));
};

uint32_t *Max::timer() { return (uint32_t *)(slot() + 0x400 + 0x1d4); }

uint8_t *Max::mural_selection() { return (uint8_t *)(slot() + 0x400 + 0x402); }

std::bitset<0xce40 * 8> *Max::map_bits(int n) {
  return (std::bitset<0xce40 * 8> *)(slot() + 0x400 + 0x404 + n * 0xce41);
}

std::array<uint8_t, 200> *Max::mural() {
  return (std::array<uint8_t, 200> *)(slot() + 0x400 + 0x26ec7);
}

Map *Max::map(int m) {
  if (m >= 0 && m <= 4)
    return (Map *)(*(size_t *)get_address("layer_base") + 0x2d0 + m * 0x1b8f84);
  return nullptr;
}

Room *Max::room(int m, int x, int y) {
  if (!rooms.contains(m)) {
    auto layer = Max::get().map(m);
    if (!layer)
      return nullptr;
    for (int r = 0; r < layer->roomCount; ++r) {
      auto *room = &layer->rooms[r];
      rooms[m][room->x | (room->y << 8)] = &layer->rooms[r];
    }
  }
  if (rooms.contains(m) && rooms[m].contains(x | (y << 8)))
    return rooms[m][x | (y << 8)];
  return nullptr;
}

Tile *Max::tile(int m, int rx, int ry, int x, int y, int l) {
  auto room = Max::room(m, rx, ry);
  if (room)
    return &room->tiles[l][y][x];
  return nullptr;
}

bool Max::import_map(std::string file, int m) {
  auto layer = Max::get().map(m);
  if (!layer)
    return false;

  MapHeader header;
  std::ifstream f;

  f.open(file.c_str(), std::ifstream::binary);
  if (!f.good())
    return false;

  f.read(reinterpret_cast<char *>(&header.signature1),
         sizeof(header.signature1));
  f.read(reinterpret_cast<char *>(&header.roomCount), sizeof(header.roomCount));
  f.read(reinterpret_cast<char *>(&header.world_wrap_x_start),
         sizeof(header.world_wrap_x_start));
  f.read(reinterpret_cast<char *>(&header.world_wrap_x_end),
         sizeof(header.world_wrap_x_end));
  f.read(reinterpret_cast<char *>(&header.idk3), sizeof(header.idk3));
  f.read(reinterpret_cast<char *>(&header.signature2),
         sizeof(header.signature2));

  // DEBUG("Room Count: {}", header.roomCount);
  layer->roomCount = header.roomCount;
  layer->world_wrap_x_start = header.world_wrap_x_start;
  layer->world_wrap_x_end = header.world_wrap_x_end;
  f.read(reinterpret_cast<char *>(&layer->rooms), 0x1b8f84);
  f.close();
  return true;
}

void Max::save_game() {
  using SaveGameFunc = void();
  static SaveGameFunc *save = (SaveGameFunc *)get_address("save_game");
  return save();
}

size_t Max::decrypt_layer(size_t asset, uint8_t *key, int layer) {
  using DecryptFunc = size_t(size_t asset, uint8_t * key, int layer);
  static DecryptFunc *decrypt = (DecryptFunc *)get_address("decrypt_layer");
  return decrypt(asset, key, layer);
}

AssetInfo *Max::decrypt_asset(uint32_t id, uint8_t *key) {
  using DecryptFunc = AssetInfo *(uint32_t id, uint8_t * key);
  static DecryptFunc *decrypt = (DecryptFunc *)get_address("decrypt_asset");
  return decrypt(id, key);
}

void Max::decrypt_stuff() {
  static bool done{false};
  if (!done) {
    done = true;
    decrypt_layer(193, (uint8_t *)&encryption_keys[0], 2); // space
    decrypt_layer(52, (uint8_t *)&encryption_keys[1], 3);  // temple
    decrypt_layer(222, (uint8_t *)&encryption_keys[2], 4); // island
    // TODO
    // decrypt_asset(30, (uint8_t *)&encryption_keys[1]); // chungus
    // decrypt_asset(277, (uint8_t *)&encryption_keys[2]); // capsule
    // decrypt_asset(377, (uint8_t *)&encryption_keys[2]); // tape
  }
}

AssetInfo *Max::get_asset(uint32_t id) {
  return (AssetInfo *)(get_address("assets") + id * sizeof(AssetInfo));
}

std::string get_custom_asset_path(uint32_t id) {
  if (!fs::exists("MAXWELL\\Mods") || !fs::is_directory("MAXWELL\\Mods"))
    return "";
  for (const auto &p : fs::recursive_directory_iterator("MAXWELL\\Mods")) {
    if (!fs::is_directory(p)) {
      auto stem = p.path().stem().string();
      auto ext = p.path().extension().string();
      auto fileId = std::atoi(stem.c_str());
      if (ext == ".map")
        continue;
      if (id == fileId && (fileId != 0 || stem == "0"))
        return p.path().string();
    }
  }
  return "";
}

void Max::load_custom_asset(uint32_t id, AssetInfo &asset) {
  std::string file = get_custom_asset_path(id);
  if (!assets.contains(id) && file != "") {
    size_t s = fs::file_size(file);
    char *buf = (char *)malloc(s);
    std::ifstream f(file.c_str(), std::ios::in | std::ios::binary);
    f.read(buf, s);
    asset.flags &= 0x3f;
    asset.data = buf;
    asset.size = s;
    assets[id] = asset;
    // DEBUG("Loaded custom asset: {} {}", id, asset.data);
  }
  if (assets.contains(id)) {
    asset = assets[id];
    // DEBUG("Found custom asset: {} {}", id, asset.data);
  }
}

void Max::load_mods() {
  if (!fs::exists("MAXWELL\\Mods") || !fs::is_directory("MAXWELL\\Mods"))
    return;
  for (const auto &p : fs::recursive_directory_iterator("MAXWELL\\Mods")) {
    if (!fs::is_directory(p)) {
      auto file = p.path().string();
      auto stem = p.path().stem().string();
      auto ext = p.path().extension().string();
      auto id = std::atoi(stem.c_str());
      if (ext == ".map") {
        import_map(file, id);
        // DEBUG("Loaded map {} from {}", id, p.path().string());
      } else if (id != 0 || stem == "0") {
        size_t s = fs::file_size(file);
        char *buf = (char *)malloc(s);
        std::ifstream f(file.c_str(), std::ios::in | std::ios::binary);
        f.read(buf, s);
        get_asset(id)->flags &= 0x3f;
        get_asset(id)->data = buf;
        get_asset(id)->size = s;
        assets[id] = *get_asset(id);
        // DEBUG("Loaded mod {} from {}", id, file);
      }
    }
  }
}

// TODO: I don't know wtf this does
void *Max::load_asset(uint32_t id, uint8_t b) {
  using LoadFunc = void *(uint32_t id, uint8_t b);
  static LoadFunc *load = (LoadFunc *)get_address("load_asset");
  auto ret = load(id, b);
  // DEBUG("LoadAsset: {} {} {}", id, b, ret);
  return ret;
}

void Max::draw_text(int x, int y, const wchar_t *text) {
  using DrawTextFunc = void(int x, int y, const wchar_t *text);
  static DrawTextFunc *draw = (DrawTextFunc *)get_address("draw_text");
  draw(x, y, text);
}
