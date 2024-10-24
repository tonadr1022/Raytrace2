#pragma once

#include <glm/ext/quaternion_float.hpp>

#include "Defs.hpp"

namespace raytrace2::cpu {

struct Ray;
struct Transform {
  vec3 translation;
  // TODO: quaternions instead
  glm::quat rotation;
  vec3 scale;

  [[nodiscard]] Ray Apply(const Ray& ray) const;
  [[nodiscard]] vec3 Apply(const vec3& point) const;
};
}  // namespace raytrace2::cpu
