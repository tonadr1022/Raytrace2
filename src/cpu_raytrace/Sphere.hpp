#pragma once

#include "Defs.hpp"
#include "Ray.hpp"
#include "cpu_raytrace/Fwd.hpp"
#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Interval.hpp"

namespace raytrace2::cpu {

struct Sphere {
  vec3 center;
  float radius;
  std::shared_ptr<MaterialVariant> material;
  bool Hit(const Ray& r, Interval ray_t, HitRecord& rec) const;
};
}  // namespace raytrace2::cpu
