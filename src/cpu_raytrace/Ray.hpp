#pragma once

namespace raytrace2::cpu {

struct Ray {
  [[nodiscard]] inline glm::dvec3 At(double t) const { return origin + direction * t; }
  glm::dvec3 origin;
  glm::dvec3 direction;
};

}  // namespace raytrace2::cpu
