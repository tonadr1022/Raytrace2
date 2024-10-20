#pragma once

#include "Defs.hpp"
#include "Interval.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

class AABB {
 public:
  AABB() = default;
  AABB(const vec3& min, const vec3& max) : min(min), max(max) { PadToMinimums(); }
  AABB(const AABB& a, const AABB& b) : min(glm::min(a.min, b.min)), max(glm::max(a.max, b.max)) {
    PadToMinimums();
  }

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

  [[nodiscard]] int LongestAxis() const {
    float x_size = max.x - min.x;
    float y_size = max.y - min.y;
    float z_size = max.z - min.z;
    if (x_size > y_size) {
      return x_size > z_size ? 0 : 2;
    }
    return y_size > z_size ? 1 : 2;
  }
  vec3 min{kInfinity}, max{-kInfinity};

 private:
  void PadToMinimums() {
    // Adjust aabb so no side is narrower than delta, padding if necessary
    constexpr const float kDelta = 0.0001f;
    constexpr const float kHalfDelta = kDelta / 2.f;
    if (std::abs(max.x - min.x) < kDelta) {
      max.x += kHalfDelta;
      min.x -= kHalfDelta;
    }
    if (std::abs(max.y - min.y) < kDelta) {
      max.y += kHalfDelta;
      min.y -= kHalfDelta;
    }
    if (std::abs(max.z - min.z) < kDelta) {
      max.z += kHalfDelta;
      min.z -= kHalfDelta;
    }
  }
};
}  // namespace raytrace2::cpu
