#include "max.h"

#include <Windows.h>
#include <array>

#include "detours.h"
#include "ghidra_byte_string.h"
#include "logger.h"
#include "memory.h"

const std::array<uint8_t, 16> encryption_keys[3] = {
    {'G', 'o', 'o', 'd', 'L', 'U', 'c', 'K', 'M', 'y', 'F', 'r', 'i', 'E', 'n',
     'd'},
    {0xC9, 0x9B, 0x64, 0x96, 0x5C, 0xCE, 0x04, 0xF0, 0xF5, 0xCB, 0x54, 0xCA,
     0xC9, 0xAB, 0x62, 0xC6},
    {0x11, 0x14, 0x18, 0x14, 0x88, 0x82, 0x42, 0x82, 0x28, 0x24, 0x88, 0x82,
     0x11, 0x18, 0x44, 0x11}};

using UpdateGamePtr = void(size_t);
static UpdateGamePtr *g_update_game_trampoline{nullptr};
void HookUpdateGame(size_t addr) { g_update_game_trampoline(addr); }

inline bool &get_is_init() {
  static bool is_init{false};
  return is_init;
}

Max &Max::get() {
  static Max MAX;
  if (!get_is_init()) {
    preload_addresses();
    {
      auto off = get_address("check");
      if (off) {
        write_mem_recoverable("check", off, "E9 24 01 00 00 90"_gh, true);
      }
    }

    {
      decrypt_layer(193, (uint8_t *)&encryption_keys[0], 2); // space
      decrypt_layer(52, (uint8_t *)&encryption_keys[1], 3);  // bunny temple
      decrypt_layer(222, (uint8_t *)&encryption_keys[2], 4); // capsule island

      // TODO
      // decrypt_asset(30, (uint8_t *)&encryption_keys[1]); // chungus
      // decrypt_asset(277, (uint8_t *)&encryption_keys[2]); // capsule
      // decrypt_asset(377, (uint8_t *)&encryption_keys[2]); // tape
    }

    /*{
      g_update_game_trampoline = (UpdateGamePtr *)get_address("update_game"sv);
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_update_game_trampoline, HookUpdateGame);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking UpdateGame: {}\n", error);
      }
    }*/
    get_is_init() = true;
  }
  return MAX;
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

int *Max::player_layer() {
  return (int *)(*(size_t *)get_address("layer_base") +
                 *(uint32_t *)get_address("layer_offset"));
}

Coord *Max::respawn_room() { return (Coord *)(player() + 0x98); }

Coord *Max::respawn_position() { return (Coord *)(player() + 0xa0); }

Coord *Max::warp_room() { return (Coord *)(player() + 0x34); }

Coord *Max::warp_position() { return (Coord *)(player() + 0x3c); }

int *Max::warp_layer() { return (int *)(player() + 0x44); }

uint8_t *Max::player_state() { return (uint8_t *)(player() + 0x5d); }

uint8_t *Max::player_flute() { return (uint8_t *)(player() + 0x8955); }

Directions *Max::player_directions() {
  return (Directions *)(player() + 0x891c);
}

int8_t *Max::player_hp() { return (int8_t *)(slot() + 0x5cc); }

Coord *Max::spawn_room() { return (Coord *)(slot() + 0x5ec); }

uint16_t *Max::equipment() { return (uint16_t *)(slot() + 0x5f4); }

uint8_t *Max::items() { return (uint8_t *)(slot() + 0x5f6); }

uint32_t *Max::upgrades() { return (uint32_t *)(slot() + 0x604); }

uint8_t *Max::keys() { return (uint8_t *)(slot() + 0x5c9); }

uint8_t *Max::options() {
  return (uint8_t *)(*(size_t *)get_address("slots") + 0x400 + 0x75048);
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

uint8_t *Max::decrypt_asset(size_t asset, uint8_t *key) {
  using DecryptFunc = uint8_t *(size_t asset, uint8_t * key);
  static DecryptFunc *decrypt = (DecryptFunc *)get_address("decrypt_asset");
  return decrypt(asset, key);
}
