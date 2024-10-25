#pragma once

#include "Defs.hpp"
namespace raytrace2::cpu {

struct Interval {
  Interval() = default;
  Interval(real min, real max) : min(min), max(max) {}
  Interval(const Interval& a, const Interval& b) {
    min = std::fmin(a.min, b.min);
    max = std::fmax(a.max, b.max);
  }

  // default is empty
  real min{kInfinity}, max{-kInfinity};
  [[nodiscard]] real Size() const { return max - min; }

  [[nodiscard]] bool Contains(real x) const { return min <= x && x <= max; }
  [[nodiscard]] bool Surrounds(real x) const { return min < x && x < max; }
  [[nodiscard]] Interval Expand(real delta) const;

  static const Interval kEmpty;
  static const Interval kUniverse;
};

inline Interval operator+(const Interval& ival, real displacement) {
  return {ival.min + displacement, ival.max + displacement};
}
inline Interval operator+(real displacement, const Interval& ival) { return ival + displacement; }

}  // namespace raytrace2::cpu
