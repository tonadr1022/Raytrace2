#pragma once

#include "Defs.hpp"
#include "Interval.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

class AABB {
 public:
  AABB() = default;
  AABB(const vec3& a, const vec3& b)
      : x(std::fmin(a.x, b.x), std::fmax(a.x, b.x)),
        y(std::fmin(a.y, b.y), std::fmax(a.y, b.y)),
        z(std::fmin(a.z, b.z), std::fmax(a.z, b.z)) {
    PadToMinimums();
  }
  AABB(const AABB& a, const AABB& b) : x(Interval{a.x, b.x}), y({a.y, b.y}), z({a.z, b.z}) {
    PadToMinimums();
  }

  [[nodiscard]] const Interval& AxisInterval(int n) const {
    const Interval* axes[3] = {&x, &y, &z};
    return *axes[n];
  }

  [[nodiscard]] vec3 GetMin() const { return vec3{x.min, y.min, z.min}; }
  [[nodiscard]] vec3 GetMax() const { return vec3{x.max, y.max, z.max}; }

  [[nodiscard]] bool Hit(const Ray& r, Interval ray_t) const {
    for (int axis = 0; axis < 3; axis++) {
      const Interval& ax = AxisInterval(axis);
      const float ad_inv = 1.f / r.direction[axis];
      auto t0 = (ax.min - r.origin[axis]) * ad_inv;
      auto t1 = (ax.max - r.origin[axis]) * ad_inv;

      if (t1 < t0) std::swap(t0, t1);
      ray_t.min = glm::max(t0, ray_t.min);
      ray_t.max = glm::min(t1, ray_t.max);
      if (ray_t.max <= ray_t.min) return false;
    }
    return true;
  }

  [[nodiscard]] int LongestAxis() const {
    if (x.Size() > y.Size()) {
      return x.Size() > z.Size() ? 0 : 2;
    }
    return y.Size() > z.Size() ? 1 : 2;
  }
  Interval x, y, z;

 private:
  void PadToMinimums() {
    // Adjust aabb so no side is narrower than delta, padding if necessary
    constexpr const float kDelta = 0.0001f;
    if (x.Size() < kDelta) x = x.Expand(kDelta);
    if (y.Size() < kDelta) y = y.Expand(kDelta);
    if (z.Size() < kDelta) z = z.Expand(kDelta);
  }
};
}  // namespace raytrace2::cpu
