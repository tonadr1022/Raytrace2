#include "Interval.hpp"

#include "Defs.hpp"

namespace raytrace2::cpu {

const Interval Interval::kEmpty = Interval{kInfinity, -kInfinity};
const Interval Interval::kUniverse = Interval{-kInfinity, kInfinity};

Interval Interval::Expand(float delta) const {
  float padding = delta / 2.0f;
  return Interval{min - padding, max + padding};
}

}  // namespace raytrace2::cpu
