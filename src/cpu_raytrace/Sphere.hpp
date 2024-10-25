#pragma once

#include "Defs.hpp"
#include "Ray.hpp"
#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Hittable.hpp"
#include "cpu_raytrace/Interval.hpp"
#include "cpu_raytrace/Transform.hpp"

namespace raytrace2::cpu {
struct Scene;

struct Sphere : public Hittable {
  Sphere(const vec3& static_center, real radius, uint32_t material_handle)
      : center_displacement(static_center, {0, 0, 0}),
        aabb(static_center - vec3(radius), static_center + vec3(radius)),
        radius(radius),
        material_handle(material_handle) {}

  Sphere(const vec3& start_position, const vec3& displacement, real radius,
         uint32_t material_handle)
      : center_displacement(start_position, displacement),
        aabb(AABB{center_displacement.At(0) - vec3{radius},
                  center_displacement.At(0) + vec3{radius}},
             AABB{center_displacement.At(1) - vec3{radius},
                  center_displacement.At(1) + vec3{radius}}),
        radius(radius),
        material_handle(material_handle) {}

  Ray center_displacement;
  AABB aabb;
  real radius;
  uint32_t material_handle;
  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb; }
  static vec2 GetUV(const vec3& p);
};

}  // namespace raytrace2::cpu

namespace raytrace2 {
struct alignas(16) GPUSphere {
  vec3 center;
  real radius;
  uint32_t material_handle;
};
}  // namespace raytrace2
