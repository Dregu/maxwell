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
        write_mem_recoverable("check", off, "E9 01 03 00 00 90"_gh, true);
      }
    }
    {
      g_update_game_trampoline = (UpdateGamePtr *)get_address("update_game"sv);
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());
      DetourAttach((void **)&g_update_game_trampoline, HookUpdateGame);
      const LONG error = DetourTransactionCommit();
      if (error != NO_ERROR) {
        DEBUG("Failed hooking UpdateGame: {}\n", error);
      }
    }
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
  return (uint8_t *)((size_t *)get_address("slots") + 0x40c);
}

Slot Max::slot() {
  return *(size_t *)get_address("slots") + SLOT_SIZE * *slot_number();
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

Coord *Max::warp_room() { return (Coord *)(player() + 0x34); }

Coord *Max::warp_position() { return (Coord *)(player() + 0x3c); }

int *Max::warp_layer() { return (int *)(player() + 0x44); }

uint8_t *Max::player_state() { return (uint8_t *)(player() + 0x5d); }

uint8_t *Max::player_flute() { return (uint8_t *)(player() + 0x8955); }

uint8_t *Max::player_hp() { return (uint8_t *)(slot() + 0x5cc); }

Coord *Max::spawn_room() { return (Coord *)(slot() + 0x5ec); }

void Max::save_game() {
  using SaveGameFunc = void();
  static SaveGameFunc *save = (SaveGameFunc *)get_address("save_game");
  return save();
}
