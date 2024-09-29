#include "Interval.hpp"

#include "Defs.hpp"

namespace raytrace2::cpu {

const Interval Interval::kEmpty = Interval{.min = kInfinity, .max = -kInfinity};
const Interval Interval::kUniverse = Interval{.min = -kInfinity, .max = kInfinity};

}  // namespace raytrace2::cpu
