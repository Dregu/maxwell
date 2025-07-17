#pragma once

#include <Windows.h>
#include <Winuser.h>
#include <array>
#include <bitset>
#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <filesystem>

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

enum class GAME_INPUT : int32_t {
  NONE = 0,
  UP,
  DOWN,
  LEFT,
  RIGHT,
  MAP,
  LB,
  RB,
  JUMP,
  ACTION,
  ITEM,
  INVENTORY,
  PAUSE,
  HUD,
  CRING,
};

enum class BUTTON_ICON : uint8_t {
  JUMP = 1,
  ACTION = 2,
  ITEM = 3,
  INVENTORY = 4,
  LB = 8,
  RB = 9,
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
static_assert(sizeof(Room) == 0x1b88);

struct RoomData {
  uint8_t bgId;
  uint8_t waterLevel;
  RoomParams params;
};

struct Map {
  uint16_t roomCount;
  uint8_t world_wrap_x_start;
  uint8_t world_wrap_x_end;
  std::array<Room, 256> rooms;
  int room_lookup[24][20];
};
static_assert(sizeof(Map) == 0x1b8f84);

struct MapHeader {
  uint32_t signature1;
  uint16_t roomCount;
  uint8_t world_wrap_x_start;
  uint8_t world_wrap_x_end;
  uint32_t idk3;
  uint32_t signature2;
};
static_assert(sizeof(MapHeader) == 16);

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
  uint8_t* data;
  uint32_t size;
  uint32_t unk1;
  size_t unk2;
  uint8_t* original_data;
  size_t unk3;
};
static_assert(sizeof(AssetInfo) == 0x30);

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

struct LightingData {
  uint8_t fg_ambient_multi[4]; // increases fg ambient light intensity
  uint8_t bg_ambient_multi[4]; // increases bg ambient light intensity
  uint8_t ambient_light[4];
  uint8_t light_intensity[4];
  float dividers[3];        // divides colors
  float saturation;         // global saturation
  float bg_tex_light_multi; // amount lights affect background texture
};

struct KangarooEncounter {
  float sack_x;
  float sack_y;
  uint8_t room_x;
  uint8_t room_y;
  uint8_t state;
  uint8_t id;
};

struct Kangaroo {
  std::array<KangarooEncounter, 3> encounter;
  uint8_t next_encounter;
  uint8_t state;
};

struct SaveData {
  uint32_t version;
  uint32_t achievements;
  uint32_t seed;
  uint8_t slot;
  uint8_t hash;
  bool edited;
  uint8_t unknown;
  uint32_t unlockables;
  // Slot slot[3];
  // Options options;
};

struct ModEntry {
  bool enabled = true;
  std::filesystem::path path;
  // does mod have overlapping assets?
  bool overlap = false;
};

// TODO: This is a horrible prototype still
struct Max {
  static Max &get();
  void unhook();

  static State state();
  Minimap minimap();
  uint8_t *slot_number();
  Slot slot();
  SaveData *save();
  Player player();
  S32Vec2 *player_room();
  FVec2 *player_position();
  FVec2 *player_velocity();
  FVec2 *player_wheel();
  FVec2 *uv_bunny();
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
  uint64_t *eggs();
  uint8_t *flames();
  uint64_t *chests();
  uint16_t *candles();
  uint32_t *bunnies();
  uint16_t *squirrels();
  Kangaroo *kangaroo();
  uint8_t *portals();
  uint8_t *shards();
  uint16_t *progress();
  uint8_t *manticore();
  Pause *pause();
  uint32_t *timer();
  uint32_t *steps();
  void save_game();
  uint8_t *mural_selection();
  std::array<uint8_t, 200> *mural();
  std::bitset<0xce40 * 8> *map_bits(int n = 0);
  Map *map(int m = 0);
  Room *room(int m, int x, int y);
  LightingData *lighting(int id);
  Tile *tile(int m, int rx, int ry, int x, int y, int l);

  bool import_map(const std::string& file, int m = 0);
  // restores original assets
  void restore_original();
  void update_mod_list();
  void reload_mods(bool preload = false);
  void load_tile_mods();
  static AssetInfo *get_asset(uint32_t id);
  static AssetInfo *decrypt_asset(uint32_t id, int key);

  // updates current room. same as exiting and entering
  void update_room();

  void load_maps_from_asset();
  std::array<uv_data, 1024> *tile_uvs();

  void draw_text_big(int x, int y, const wchar_t *text);
  void draw_text_small(int x, int y, const wchar_t *text, uint32_t color = 0xffffffff, uint32_t shader = 0x29);

  uint16_t get_room_tile_flags(int x, int y, uint16_t mask);

  void dump_lighting();
  void dump_map(uint8_t m = 0);
  void dump_asset(uint32_t id);

  bool skip{false};
  std::optional<bool> paused{std::nullopt};
  std::optional<bool> set_pause{std::nullopt};

  PLAYER_INPUT input{PLAYER_INPUT::SKIP};
  std::deque<int> inputs;
  std::deque<std::function<void()>> render_queue;

  std::optional<uint8_t> force_water{std::nullopt};
  std::unordered_map<GAME_INPUT, uint8_t> keymap{
      {GAME_INPUT::UP, VK_UP},     {GAME_INPUT::DOWN, VK_DOWN},
      {GAME_INPUT::LEFT, VK_LEFT}, {GAME_INPUT::RIGHT, VK_RIGHT},
      {GAME_INPUT::ACTION, 'A'},   {GAME_INPUT::LB, 'S'},
      {GAME_INPUT::RB, 'D'},       {GAME_INPUT::JUMP, 'Z'},
      {GAME_INPUT::ITEM, 'X'},     {GAME_INPUT::INVENTORY, 'C'},
      {GAME_INPUT::MAP, 'V'},      {GAME_INPUT::PAUSE, VK_ESCAPE},
      {GAME_INPUT::HUD, 'H'},      {GAME_INPUT::CRING, 'F'},
  };

  bool atlas_loaded{false};

  std::unordered_map<std::string, ModEntry> mods;
};
