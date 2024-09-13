#pragma once

#include "Defs.hpp"
namespace raytrace2::cpu {

struct Interval {
  double min{-kInfinity}, max{kInfinity};
  [[nodiscard]] double Size() const { return max - min; }

  [[nodiscard]] bool Contains(double x) const { return min <= x && x <= max; }
  [[nodiscard]] bool Surrounds(double x) const { return min < x && x < max; }

  static const Interval kEmpty;
  static const Interval kUniverse;
};

}  // namespace raytrace2::cpu
