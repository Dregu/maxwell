#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <span>
#include <string>

#include "image.h"

#define SLOT_SIZE 0x27010

using State = size_t;
using Minimap = size_t;
using Slot = size_t;
using Player = size_t;
using Options = size_t;

#define ENUM_CLASS_FLAGS(Enum)                                                 \
  inline constexpr Enum operator|(Enum Lhs, Enum Rhs) {                        \
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) |  \
                             static_cast<std::underlying_type_t<Enum>>(Rhs));  \
  }                                                                            \
  inline constexpr Enum operator&(Enum Lhs, Enum Rhs) {                        \
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) &  \
                             static_cast<std::underlying_type_t<Enum>>(Rhs));  \
  }                                                                            \
  inline constexpr Enum operator^(Enum Lhs, Enum Rhs) {                        \
    return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) ^  \
                             static_cast<std::underlying_type_t<Enum>>(Rhs));  \
  }                                                                            \
  inline constexpr Enum operator~(Enum E) {                                    \
    return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(E));   \
  }                                                                            \
  inline Enum &operator|=(Enum &Lhs, Enum Rhs) {                               \
    return Lhs = static_cast<Enum>(                                            \
               static_cast<std::underlying_type_t<Enum>>(Lhs) |                \
               static_cast<std::underlying_type_t<Enum>>(Rhs));                \
  }                                                                            \
  inline Enum &operator&=(Enum &Lhs, Enum Rhs) {                               \
    return Lhs = static_cast<Enum>(                                            \
               static_cast<std::underlying_type_t<Enum>>(Lhs) &                \
               static_cast<std::underlying_type_t<Enum>>(Rhs));                \
  }                                                                            \
  inline Enum &operator^=(Enum &Lhs, Enum Rhs) {                               \
    return Lhs = static_cast<Enum>(                                            \
               static_cast<std::underlying_type_t<Enum>>(Lhs) ^                \
               static_cast<std::underlying_type_t<Enum>>(Rhs));                \
  }

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

struct Tile {
  uint16_t id;
  uint8_t param;
  uint8_t flags;
};

struct RoomParams {
  uint8_t palette;
  uint8_t idk1;
  uint8_t idk2;
  uint8_t idk3;
};

struct Room {
  uint8_t x;
  uint8_t y;
  uint8_t bgId;
  uint8_t waterLevel;

  RoomParams params;

  Tile tiles[2][22][40];
};

struct Map {
  uint16_t roomCount;
  uint8_t world_wrap_x_start;
  uint8_t world_wrap_x_end;
  std::array<Room, 256> rooms;
};

struct MapHeader {
  uint32_t signature1;
  uint16_t roomCount;
  uint8_t world_wrap_x_start;
  uint8_t world_wrap_x_end;
  uint32_t idk3;
  uint32_t signature2;
};

enum class AssetType : uint8_t {
  Text = 0,
  Binary = 1,
  Png = 2,
  Ogg = 3,
  SpriteData = 5,
  Shader = 7,
  Font = 8,

  Normal = 0x3F,
  Encrypted = 0x40,
  Decrypted = 0x80,
};
ENUM_CLASS_FLAGS(AssetType);

struct AssetInfo {
  AssetType type;
  void *data;
  uint32_t size;
  uint32_t unk1;
  size_t unk2;
  void *original_data;
  size_t unk3;
};

struct uv_data {
  U16Vec2 pos;
  U16Vec2 size;

  union {
    struct {
      bool collides_left : 1;
      bool collides_right : 1;
      bool collides_up : 1;
      bool collides_down : 1;

      bool not_placeable : 1;
      bool additive : 1;

      bool obscures : 1;
      bool contiguous : 1;
      bool blocks_light : 1;
      bool self_contiguous : 1;
      bool hidden : 1;
      bool dirt : 1;
      bool has_normals : 1;
      bool uv_light : 1;
    };
    uint16_t flags;
  };
};
static_assert(sizeof(uv_data) == 10);

// TODO: This is a horrible prototype still
struct Max {
  static Max &get();
  void unhook();
  static State state();
  Minimap minimap();
  uint8_t *slot_number();
  Slot slot();
  Player player();
  S32Vec2 *player_room();
  FVec2 *player_position();
  FVec2 *player_velocity();
  FVec2 *player_wheel();
  int *player_map();
  S32Vec2 *respawn_room();
  S32Vec2 *respawn_position();
  S32Vec2 *warp_room();
  S32Vec2 *warp_position();
  int *warp_map();
  uint8_t *player_flute();
  Directions *player_directions();
  uint8_t *player_state();
  int8_t *player_hp();
  S32Vec2 *spawn_room();
  uint16_t *equipment();
  uint8_t *items();
  uint32_t *upgrades();
  uint8_t *keys();
  uint8_t *item();
  uint8_t *options();
  Pause *pause();
  uint32_t *timer();
  void save_game();
  uint8_t *mural_selection();
  std::array<uint8_t, 200> *mural();
  std::bitset<0xce40 * 8> *map_bits(int n = 0);
  Map *map(int m = 0);
  Room *room(int m, int x, int y);
  Tile *tile(int m, int rx, int ry, int x, int y, int l);
  bool import_map(std::string file, int m = 0);
  void load_custom_asset(uint32_t id, AssetInfo &asset);
  void load_map_mods();
  void load_tile_mods(AssetInfo &asset);
  static AssetInfo *get_asset(uint32_t id);
  static size_t decrypt_layer(size_t asset, uint8_t *key, int layer);
  static AssetInfo *decrypt_asset(uint32_t id, uint8_t *key);
  static void *load_asset(uint32_t id, uint8_t b);
  std::array<uv_data, 1024> *tile_uvs();
  void decrypt_stuff();

  void draw_text(int x, int y, const wchar_t *text);

  bool skip{false};
  std::optional<bool> paused{std::nullopt};
  std::optional<bool> set_pause{std::nullopt};

  PLAYER_INPUT input{PLAYER_INPUT::SKIP};
  std::deque<int> inputs;
  std::deque<std::function<void()>> render_queue;

  std::optional<uint8_t> force_palette{std::nullopt};
  std::optional<uint8_t> force_water{std::nullopt};
  std::unordered_map<int, AssetInfo> assets;

  bool atlas_loaded{false};
};
