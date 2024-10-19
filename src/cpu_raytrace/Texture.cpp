#include "Texture.hpp"

#include "Defs.hpp"

namespace raytrace2::cpu::texture {

vec3 Checker::Value(const TexArray& tex_arr, vec2 uv, const vec3& p) const {
  glm::ivec3 i = glm::floor(inv_scale * p);
  const TextureVariant& t = tex_arr[(i.x + i.y + i.z) % 2 == 0 ? even_tex_idx : odd_tex_idx];
  return std::visit([&uv, &p, &tex_arr](auto&& tex) { return tex.Value(tex_arr, uv, p); }, t);
}

}  // namespace raytrace2::cpu::texture
