#pragma once

#include "Defs.hpp"
#include "Interval.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

class AABB {
 public:
  AABB() = default;
  AABB(const vec3& min, const vec3& max) : min(min), max(max) {}
  AABB(const AABB& a, const AABB& b) : min(glm::min(a.min, b.min)), max(glm::max(a.max, b.max)) {}

  [[nodiscard]] bool Hit(const Ray& r, Interval ray_t) const {
    for (int axis = 0; axis < 3; axis++) {
      if (r.direction[axis] == 0.0f) {
        if (r.origin[axis] < min[axis] || r.origin[axis] > max[axis]) {
          return false;
        }
        continue;
      }
      float inv_direction = 1.0f / r.direction[axis];
      float t0 = (min[axis] - r.origin[axis]) * inv_direction;
      float t1 = (max[axis] - r.origin[axis]) * inv_direction;
      if (t1 < t0) std::swap(t0, t1);
      ray_t.min = glm::max(t0, ray_t.min);
      ray_t.max = glm::min(t1, ray_t.max);
      if (ray_t.max <= ray_t.min) return false;
    }
    return true;
  }

  vec3 min{kInfinity}, max{-kInfinity};
};

}  // namespace raytrace2::cpu
