#pragma once

#include <cstdint>
#include <deque>
#include <optional>

#define SLOT_SIZE 0x27010

using State = size_t;
using Minimap = size_t;
using Slot = size_t;
using Player = size_t;
using Options = size_t;

enum PLAYER_INPUT : int32_t {
  SKIP = -1,
  NONE = 0,
  UP = 0x1,
  DOWN = 0x2,
  LEFT = 0x4,
  RIGHT = 0x8,
  MAP = 0x20,
  LB = 0x100,
  RB = 0x200,
  JUMP = 0x1000,
  ACTION = 0x2000,
  ITEM = 0x4000,
  INVENTORY = 0x8000,
};

struct Coord {
  int x;
  int y;
};

struct fCoord {
  float x;
  float y;
};

struct Directions {
  uint8_t buffer[32];
  uint8_t count;
};

struct Pause {
  bool paused;
  uint32_t timer;
  uint32_t index;
  uint32_t timer2;
  uint32_t unk;
  float marker[8];
  uint32_t unk2;
  uint32_t unk3;
  uint32_t type;
  uint32_t timer3;
  bool closing;
};

// TODO: This is a horrible prototype still
struct Max {
  static Max &get();
  void unhook();
  static State state();
  Minimap minimap();
  uint8_t *slot_number();
  Slot slot();
  Player player();
  Coord *player_room();
  fCoord *player_position();
  fCoord *player_velocity();
  fCoord *player_wheel();
  int *player_layer();
  Coord *respawn_room();
  Coord *respawn_position();
  Coord *warp_room();
  Coord *warp_position();
  int *warp_layer();
  uint8_t *player_flute();
  Directions *player_directions();
  uint8_t *player_state();
  int8_t *player_hp();
  Coord *spawn_room();
  uint16_t *equipment();
  uint8_t *items();
  uint32_t *upgrades();
  uint8_t *keys();
  uint8_t *item();
  uint8_t *options();
  Pause *pause();
  uint32_t *timer();
  void save_game();
  static size_t decrypt_layer(size_t asset, uint8_t *key, int layer);
  static uint8_t *decrypt_asset(size_t asset, uint8_t *key);

  bool skip{false};
  std::optional<bool> paused{std::nullopt};
  std::optional<bool> set_pause{std::nullopt};

  PLAYER_INPUT input{PLAYER_INPUT::SKIP};
  std::deque<int> inputs;
};
