using State = size_t;
using Minimap = size_t;

struct Max {
  static Max &get();
  static State state();
  Minimap minimap();
};
