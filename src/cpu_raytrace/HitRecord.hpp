#pragma once

#include "Defs.hpp"
#include "Fwd.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

struct HitRecord {
  vec3 point;
  vec3 normal;
  float t;
  const MaterialVariant* material{};
  bool front_face;

  inline void SetFaceNormal(const Ray& r, const vec3& outward_normal) {
    front_face = glm::dot(r.direction, outward_normal) < 0;
    normal = (static_cast<float>((static_cast<int>(front_face)) << 1) - 1.0f) * outward_normal;
  }
};
}  // namespace raytrace2::cpu
