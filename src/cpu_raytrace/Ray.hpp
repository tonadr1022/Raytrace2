#pragma once

namespace raytrace2::cpu {

struct Ray {
  [[nodiscard]] inline glm::dvec3 At(float t) const { return origin + direction * t; }
  glm::vec3 origin;
  glm::vec3 direction;
};

}  // namespace raytrace2::cpu
