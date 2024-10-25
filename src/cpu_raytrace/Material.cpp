#include "Material.hpp"

#include "cpu_raytrace/Fwd.hpp"
#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Math.hpp"
#include "cpu_raytrace/Texture.hpp"

namespace raytrace2::cpu {

bool MaterialMetal::Scatter(const texture::TexArray&, const Ray& r_in, const HitRecord& rec,
                            vec3& attenuation, Ray& scattered) const {
  vec3 reflected =
      glm::normalize(math::Reflect(r_in.direction, rec.normal)) + (fuzz * math::RandUnitVec3());
  scattered = Ray{.origin = rec.point, .direction = reflected, .time = r_in.time};
  attenuation = albedo;
  return true;
}

namespace {

auto SchlickReflectance(auto cosine, auto refraction_index) {
  auto r0 = (1 - refraction_index) / (1 + refraction_index);
  r0 = r0 * r0;
  return r0 + (1 - r0) * glm::pow((1 - cosine), 5);
}

}  // namespace

bool MaterialDielectric::Scatter(const texture::TexArray&, const Ray& r_in, const HitRecord& rec,
                                 vec3& attenuation, Ray& scattered) const {
  attenuation = vec3(1.0f);
  real ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;
  vec3 unit_dir = glm::normalize(r_in.direction);
  real cos_theta = glm::min(glm::dot(-unit_dir, rec.normal), static_cast<real>(1.0));
  real sin_theta = glm::sqrt(1.f - cos_theta * cos_theta);
  bool cannot_refract = ri * sin_theta > 1.0;
  vec3 direction;
  if (cannot_refract || SchlickReflectance(cos_theta, ri) > math::RandReal()) {
    direction = math::Reflect(unit_dir, rec.normal);
  } else {
    direction = math::Refract(unit_dir, rec.normal, ri);
  }
  scattered = Ray{.origin = rec.point, .direction = direction, .time = r_in.time};
  return true;
}

bool MaterialLambertian::Scatter(const texture::TexArray&, const Ray& r_in, const HitRecord& rec,
                                 vec3& attenuation, Ray& scattered) const {
  vec3 scattered_dir = rec.normal + math::RandUnitVec3();
  if (math::NearZero(scattered_dir)) {
    scattered_dir = rec.normal;
  }
  scattered = Ray{.origin = rec.point, .direction = scattered_dir, .time = r_in.time};
  attenuation = albedo;
  return true;
}

bool MaterialTexture::Scatter(const texture::TexArray& tex_arr, const Ray& r_in,
                              const HitRecord& rec, vec3& attenuation, Ray& scattered) const {
  vec3 scattered_dir = rec.normal + math::RandUnitVec3();
  if (math::NearZero(scattered_dir)) {
    scattered_dir = rec.normal;
  }
  scattered = Ray{.origin = rec.point, .direction = scattered_dir, .time = r_in.time};
  attenuation = std::visit(
      [&rec, &tex_arr](auto&& tex) -> vec3 { return tex.Value(tex_arr, rec.uv, rec.point); },
      tex_arr[tex_idx]);
  return true;
}

vec3 DiffuseLight::Emit(const texture::TexArray& tex_arr, const vec2& uv, const vec3& p) const {
  return std::visit([&tex_arr, &uv, &p](auto&& tex) -> vec3 { return tex.Value(tex_arr, uv, p); },
                    tex_arr[tex_idx]);
}

}  // namespace raytrace2::cpu
