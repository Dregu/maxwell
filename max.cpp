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

using GetInput = uint32_t(uint16_t a);
GetInput *g_get_input_trampoline{nullptr};
uint32_t HookGetInput(uint16_t a) {
  auto ret = g_get_input_trampoline(a);
  if (ret > 0)
    DEBUG("GetInput: {:x} {:x}", a, ret);

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
    else if (a == PLAYER_INPUT::LB)
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
