#pragma once

#include "Defs.hpp"
namespace raytrace2::cpu {

struct Ray {
  [[nodiscard]] inline vec3 At(float t) const { return origin + direction * t; }
  vec3 origin;
  vec3 direction;
  float time;
};

}  // namespace raytrace2::cpu
