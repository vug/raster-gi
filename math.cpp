#include "math.hpp"

constexpr Mat4 lookAt(const Vec3& camPos, const Vec3& target) {
  Mat4 translate;
  translate.wx = -camPos.x;
  translate.wy = -camPos.y;
  translate.wz = -camPos.z;


  return translate;
}