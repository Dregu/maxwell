#include "max.h"

#include <Windows.h>

#include "detours.h"
#include "ghidra_byte_string.h"
#include "logger.h"
#include "memory.h"

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
