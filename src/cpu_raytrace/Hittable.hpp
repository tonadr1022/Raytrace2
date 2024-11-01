#pragma once

#include "cpu_raytrace/AABB.hpp"
namespace raytrace2::cpu {

struct Scene;
struct Ray;
struct Interval;
struct HitRecord;

struct Hittable {
  virtual ~Hittable() = default;

  virtual bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const = 0;

  [[nodiscard]] virtual real PDFValue([[maybe_unused]] const vec3& origin,
                                      [[maybe_unused]] const vec3& dir) const {
    return 0.0;
  }

  [[nodiscard]] virtual vec3 Random([[maybe_unused]] const vec3& origin) const {
    return vec3{1, 0, 0};
  }

  [[nodiscard]] virtual AABB GetAABB() const = 0;
};

}  // namespace raytrace2::cpu
