#include <cstdint>

#define SLOT_SIZE 0x27010

using State = size_t;
using Minimap = size_t;
using Slot = size_t;

struct Max {
  static Max &get();
  static State state();
  Minimap minimap();
  uint8_t slot_number();
  Slot slot();
};
