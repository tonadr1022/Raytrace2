#include "Texture.hpp"

#include "Defs.hpp"

namespace raytrace2::cpu::texture {

vec3 Checker::Value(const TexArray& tex_arr, vec2 uv, const vec3& p) const {
  glm::ivec3 i = glm::floor(inv_scale * p);
  const TextureVariant& t = tex_arr[(i.x + i.y + i.z) % 2 == 0 ? even_tex_idx : odd_tex_idx];
  return std::visit([&uv, &p, &tex_arr](auto&& tex) { return tex.Value(tex_arr, uv, p); }, t);
}

vec3 Noise::Value(const TexArray&, vec2, const vec3& p) const {
  switch (noise_type) {
    case NoiseType::kMarble:
      return albedo * static_cast<real>(0.5) * (1 + std::sin(scale * p.z + 10 * noise.Turb(p)));
      // return vec3(1, 1, 1) * noise.Turb(p, 7);
    case NoiseType::kPerlin:
      return albedo * static_cast<real>(0.5) * (1.0f + noise.Noise(scale * p));
  }
  return {};
}

}  // namespace raytrace2::cpu::texture
