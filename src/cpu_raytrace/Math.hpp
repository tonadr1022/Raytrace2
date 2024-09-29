#pragma once

#include <random>

#include "Defs.hpp"

namespace raytrace2::cpu::math {

inline float RandFloat() {
  static std::uniform_real_distribution<float> distribution(0.0, 1.0);
  thread_local static std::minstd_rand generator(std::random_device{}());  // Faster generator
  return distribution(generator);
}

inline float RandFloat(float min, float max) { return min + RandFloat() * (max - min); }

inline vec3 RandVec3() { return {RandFloat(), RandFloat(), RandFloat()}; }

inline vec3 RandVec3(float min, float max) {
  return {RandFloat(min, max), RandFloat(min, max), RandFloat(min, max)};
}

inline vec3 RandInUnitSphere() {
  while (true) {
    vec3 p = RandVec3(-1, 1);
    float length_sq = glm::dot(p, p);
    if (1e-160 < length_sq && length_sq <= 1.0) return p;
  }
}

inline vec3 RandInUnitDisk() {
  while (true) {
    auto p = vec3(RandFloat(-1, 1), RandFloat(-1, 1), 0);
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

inline float LinearToGamma(float linear_component) {
  return std::sqrt(std::max(linear_component, 0.0f));
}

inline vec3 LinearToGamma(const vec3& linear) {
  return {LinearToGamma(linear.x), LinearToGamma(linear.y), LinearToGamma(linear.z)};
}

inline bool NearZero(const vec3& v) {
  constexpr auto kEpsilon = 1e-8;
  return (std::fabs(v.x) < kEpsilon) && (std::fabs(v.y) < kEpsilon) && (std::fabs(v.z) < kEpsilon);
}

inline vec3 Reflect(const vec3& v, const vec3& n) { return v - 2 * glm::dot(v, n) * n; }

inline vec3 Refract(const vec3& uv, const vec3& n, float etai_over_etat) {
  float cos_theta = std::fmin(glm::dot(-uv, n), 1.0);
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
  vec3 r_out_parallel = -std::sqrt(std::fabs(1.0f - glm::dot(r_out_perp, r_out_perp))) * n;
  return r_out_perp + r_out_parallel;
}

}  // namespace raytrace2::cpu::math
