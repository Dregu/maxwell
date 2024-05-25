#include "max.h"

#include "ghidra_byte_string.h"
#include "memory.h"

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

uint8_t Max::slot_number() {
  return memory_read<uint8_t>(*(size_t *)get_address("slots") + 0x40c);
}

Slot Max::slot() {
  return *(size_t *)get_address("slots") + SLOT_SIZE * slot_number();
}

Player Max::player() {
  return (Player)(*(size_t *)get_address("slots") + 0x93670);
}

Coord *Max::player_room() { return (Coord *)(player() + 0x20); }

fCoord *Max::player_position() { return (fCoord *)(player()); }

Coord *Max::warp_room() { return (Coord *)(player() + 0x34); }

Coord *Max::warp_position() { return (Coord *)(player() + 0x3c); }

int *Max::player_layer() { return (int *)(player() + 0x44); }

uint8_t *Max::player_flute() { return (uint8_t *)(player() + 0x8955); }

/*      static uint8_t* flute = (uint8_t*)(player + 0x8955);
        static Coord* room = (Coord*)(player + 0x34);
        static Coord* pos = (Coord*)(player + 0x3c);
        static fCoord* ppos = (fCoord*)(player);
        static Coord* rpos = (Coord*)(player + 0x20);
        static int* layer = (int*)(player + 0x44);*/
