#pragma once

#include <cstdint>

#define SLOT_SIZE 0x27010

using State = size_t;
using Minimap = size_t;
using Slot = size_t;
using Player = size_t;
using Options = size_t;

struct Coord {
  int x;
  int y;
};

struct fCoord {
  float x;
  float y;
};

// TODO: This is a horrible prototype still
struct Max {
  static Max &get();
  static State state();
  Minimap minimap();
  uint8_t *slot_number();
  Slot slot();
  Player player();
  Coord *player_room();
  fCoord *player_position();
  fCoord *player_velocity();
  int *player_layer();
  Coord *respawn_room();
  Coord *respawn_position();
  Coord *warp_room();
  Coord *warp_position();
  int *warp_layer();
  uint8_t *player_flute();
  uint8_t *player_state();
  int8_t *player_hp();
  Coord *spawn_room();
  uint16_t *equipment();
  uint8_t *items();
  uint32_t *upgrades();
  uint8_t *keys();
  uint8_t *options();
  void save_game();
};
