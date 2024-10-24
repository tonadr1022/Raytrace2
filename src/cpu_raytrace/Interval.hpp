#pragma once

#include "Defs.hpp"
namespace raytrace2::cpu {

struct Interval {
  Interval() = default;
  Interval(float min, float max) : min(min), max(max) {}
  Interval(const Interval& a, const Interval& b) {
    min = std::fmin(a.min, b.min);
    max = std::fmax(a.max, b.max);
  }

  // default is empty
  float min{kInfinity}, max{-kInfinity};
  [[nodiscard]] float Size() const { return max - min; }

  [[nodiscard]] bool Contains(float x) const { return min <= x && x <= max; }
  [[nodiscard]] bool Surrounds(float x) const { return min < x && x < max; }
  [[nodiscard]] Interval Expand(float delta) const;

  static const Interval kEmpty;
  static const Interval kUniverse;
};

}  // namespace raytrace2::cpu
