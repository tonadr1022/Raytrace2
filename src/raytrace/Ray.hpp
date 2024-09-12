#pragma once

struct Ray {
  [[nodiscard]] inline glm::dvec3 At(double t) const { return origin + direction * t; }
  glm::dvec3 origin;
  glm::dvec3 direction;
};
