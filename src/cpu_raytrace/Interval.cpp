#include "Interval.hpp"

#include "Defs.hpp"

namespace raytrace2::cpu {

const Interval Interval::kEmpty = Interval{.min = kInfinity, .max = -kInfinity};
const Interval Interval::kUniverse = Interval{.min = -kInfinity, .max = kInfinity};

Interval Interval::Expand(float delta) const {
  float padding = delta / 2.0f;
  return Interval{.min = min - padding, .max = max + padding};
}

}  // namespace raytrace2::cpu
