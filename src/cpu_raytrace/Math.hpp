#pragma once

#include <random>

#include "Defs.hpp"

namespace raytrace2::cpu::math {

inline real RandReal() {
  static std::uniform_real_distribution<real> distribution(0.0, 1.0);
  thread_local static std::minstd_rand generator(std::random_device{}());  // Faster generator
  return distribution(generator);
}

inline real RandReal(real min, real max) { return min + RandReal() * (max - min); }

// inclusive of min, inclusive of max
inline int RandInt(int min, int max) { return static_cast<int>(RandReal(min, max + 1)); }

inline vec3 RandVec3() { return {RandReal(), RandReal(), RandReal()}; }

inline vec3 RandVec3(real min, real max) {
  return {RandReal(min, max), RandReal(min, max), RandReal(min, max)};
}

inline vec3 RandInUnitSphere() {
  while (true) {
    vec3 p = RandVec3(-1, 1);
    real length_sq = glm::dot(p, p);
    if (1e-160 < length_sq && length_sq <= 1.0) return p;
  }
}

inline vec3 RandInUnitDisk() {
  while (true) {
    auto p = vec3(RandReal(-1, 1), RandReal(-1, 1), 0);
    if (glm::dot(p, p) < 1.0) {
      return p;
    }
  }
}

inline vec3 RandUnitVec3() { return glm::normalize(RandInUnitSphere()); }

inline vec3 RandOnHemisphere(const vec3& normal) {
  vec3 unit_vec = RandUnitVec3();
  if (glm::dot(normal, unit_vec) > 0.0) {
    return unit_vec;
  }
  return -unit_vec;
}

inline real LinearToGamma(real linear_component) {
  return std::sqrt(std::max(linear_component, static_cast<real>(0)));
}

inline vec3 LinearToGamma(const vec3& linear) {
  return {LinearToGamma(linear.x), LinearToGamma(linear.y), LinearToGamma(linear.z)};
}

inline bool NearZero(const vec3& v) {
  constexpr auto kEpsilon = 1e-8;
  return (std::fabs(v.x) < kEpsilon) && (std::fabs(v.y) < kEpsilon) && (std::fabs(v.z) < kEpsilon);
}

inline vec3 Reflect(const vec3& v, const vec3& n) { return v - 2 * glm::dot(v, n) * n; }

inline vec3 Refract(const vec3& uv, const vec3& n, real etai_over_etat) {
  real cos_theta = std::fmin(glm::dot(-uv, n), 1.0);
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
  vec3 r_out_parallel = -std::sqrt(std::fabs(1.0f - glm::dot(r_out_perp, r_out_perp))) * n;
  return r_out_perp + r_out_parallel;
}

inline ONBVecs GetONB(const vec3& n) {
  ONBVecs onb;
  onb[2] = glm::normalize(n);
  bool close_to_1 = std::fabs(onb[2].x) > 0.9;
  vec3 a = {static_cast<int>(!close_to_1), static_cast<int>(close_to_1), 0};
  onb[1] = glm::normalize(glm::cross(onb[2], a));
  onb[0] = glm::cross(onb[2], onb[1]);

  return onb;
}

inline vec3 Transform(const ONBVecs& axis, const vec3& v) {
  return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
}

inline vec3 RandomCosineDirection() {
  auto r1 = RandReal();
  auto r2 = RandReal();
  auto phi = 2 * std::numbers::pi * r1;
  auto x = std::cos(phi) * std::sqrt(r2);
  auto y = std::sin(phi) * std::sqrt(r2);
  auto z = std::sqrt(1 - r2);
  return {x, y, z};
}

}  // namespace raytrace2::cpu::math
