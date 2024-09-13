#pragma once

#include "Defs.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

struct HitRecord {
  vec3 point;
  vec3 normal;
  double t;
  bool front_face;

  inline void SetFaceNormal(const Ray& r, const vec3& outward_normal) {
    front_face = glm::dot(r.direction, outward_normal) < 0;
    normal = (static_cast<double>((static_cast<int>(front_face)) << 1) - 1.0) * outward_normal;
  }
};
}  // namespace raytrace2::cpu
