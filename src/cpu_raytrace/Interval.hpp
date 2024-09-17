#pragma once

#include "Defs.hpp"
namespace raytrace2::cpu {

struct Interval {
  float min{-kInfinity}, max{kInfinity};
  [[nodiscard]] float Size() const { return max - min; }

  [[nodiscard]] bool Contains(float x) const { return min <= x && x <= max; }
  [[nodiscard]] bool Surrounds(float x) const { return min < x && x < max; }

  static const Interval kEmpty;
  static const Interval kUniverse;
};

}  // namespace raytrace2::cpu
