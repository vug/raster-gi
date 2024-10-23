#pragma once

struct Vec3 {
  float x{};
  float y{};
  float z{};

  void normalize() {
    
  }
};

struct Mat4 {
  float xx{1};
  float xy{};
  float xz{};
  float xw{};

  float yx{};
  float yy{1};
  float yz{};
  float yw{};

  float zx{};
  float zy{};
  float zz{1};
  float zw{};

  float wx{};
  float wy{};
  float wz{};
  float ww{1};
};

constexpr Mat4 lookAt(const Vec3& camPos, const Vec3& target);
