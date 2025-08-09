#pragma once

#include <span>
#include <string>
#include <vector>

struct S32Vec2 {
  int x;
  int y;
};

struct FVec2 {
  float x;
  float y;
};

struct U16Vec2 {
  uint16_t x;
  uint16_t y;
};

class Image {
  uint32_t *data_ = nullptr;
  int width = 0, height = 0;

public:
  Image() {}
  Image(std::span<const uint8_t> data);
  Image(const std::string &path);
  Image(int width, int height);
  Image(S32Vec2 size) : Image(size.x, size.y) {}
  Image(const char *data, size_t size);

  Image(Image &&other) noexcept;
  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  ~Image();

  void save_png(const std::string &path);
  const void *get_png(int *len);

  S32Vec2 size() const { return {width, height}; }
  uint32_t *data() { return data_; }
  const uint32_t *data() const { return data_; }

  uint32_t operator()(int x, int y) const { return data_[x + y * width]; }
  uint32_t &operator()(int x, int y) { return data_[x + y * width]; }

  bool operator==(const Image &other) const {
    if (width != other.width || height != other.height)
      return false;
    for (int i = 0; i < width * height; ++i) {
      if (data_[i] != other.data_[i])
        return false;
    }
    return true;
  }

  static void save_png_from_data(const std::string &path, const void *data,
                                 int w, int h);
};
