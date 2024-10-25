#include "Interval.hpp"

#include "Defs.hpp"

namespace raytrace2::cpu {

const Interval Interval::kEmpty = Interval{kInfinity, -kInfinity};
const Interval Interval::kUniverse = Interval{-kInfinity, kInfinity};

Interval Interval::Expand(real delta) const {
  real padding = delta / 2.0f;
  return Interval{min - padding, max + padding};
}

}  // namespace raytrace2::cpu
