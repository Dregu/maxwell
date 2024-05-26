#include <cstdint>

#define SLOT_SIZE 0x27010

using State = size_t;
using Minimap = size_t;
using Slot = size_t;
using Player = size_t;

struct Coord {
  int x;
  int y;
};

struct fCoord {
  float x;
  float y;
};

struct Max {
  static Max &get();
  static State state();
  Minimap minimap();
  uint8_t slot_number();
  Slot slot();
  Player player();
  Coord *player_room();
  fCoord *player_position();
  fCoord *player_velocity();
  Coord *warp_room();
  Coord *warp_position();
  int *player_layer();
  uint8_t *player_flute();
  uint8_t *player_state();
};
