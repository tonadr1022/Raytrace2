#pragma once

#include "Defs.hpp"
#include "Ray.hpp"
#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Interval.hpp"

namespace raytrace2::cpu {
struct Scene;

struct Sphere {
  vec3 center;
  float radius;
  uint32_t material_handle;
  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const;
};
}  // namespace raytrace2::cpu
namespace raytrace2 {

struct alignas(16) GPUSphere {
  vec3 center;
  float radius;
  uint32_t material_handle;
};
}  // namespace raytrace2
