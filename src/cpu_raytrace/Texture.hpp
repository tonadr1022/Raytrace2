#pragma once

#include <glm/ext/vector_int3.hpp>

#include "Defs.hpp"
#include "cpu_raytrace/Fwd.hpp"
#include "cpu_raytrace/PerlinNoiseGen.hpp"

namespace raytrace2::cpu {
struct Scene;
}
namespace raytrace2::cpu::texture {

struct SolidColor {
  [[nodiscard]] inline vec3 Value(const TexArray&, vec2, const vec3&) const { return albedo; }
  vec3 albedo;
};

struct Checker {
  Checker(real scale, uint32_t even_tex_idx, uint32_t odd_tex_idx)
      : inv_scale(1.f / scale), even_tex_idx(even_tex_idx), odd_tex_idx(odd_tex_idx) {}
  Checker() = default;

  [[nodiscard]] vec3 Value(const TexArray& tex_arr, vec2 uv, const vec3& p) const;
  real inv_scale{1};
  uint32_t even_tex_idx{};
  uint32_t odd_tex_idx{};
};

enum class NoiseType { kPerlin, kMarble };
struct Noise {
  [[nodiscard]] vec3 Value(const TexArray& tex_arr, vec2 uv, const vec3& p) const;
  PerlinNoiseGen noise;
  vec3 albedo{1};
  real scale{1};
  NoiseType noise_type{NoiseType::kMarble};

 private:
};

}  // namespace raytrace2::cpu::texture
