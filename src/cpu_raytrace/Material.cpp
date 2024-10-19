#include "Material.hpp"

#include "cpu_raytrace/Fwd.hpp"
#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Math.hpp"
#include "cpu_raytrace/Texture.hpp"

namespace raytrace2::cpu {

bool Scatter(const texture::TexArray&, const MaterialMetal& mat, const Ray& r_in,
             const HitRecord& rec, vec3& attenuation, Ray& scattered) {
  vec3 reflected =
      glm::normalize(math::Reflect(r_in.direction, rec.normal)) + (mat.fuzz * math::RandUnitVec3());
  scattered = Ray{.origin = rec.point, .direction = reflected, .time = r_in.time};
  attenuation = mat.albedo;
  return true;
}

namespace {

auto SchlickReflectance(auto cosine, auto refraction_index) {
  auto r0 = (1 - refraction_index) / (1 + refraction_index);
  r0 = r0 * r0;
  return r0 + (1 - r0) * glm::pow((1 - cosine), 5);
}

}  // namespace

bool Scatter(const texture::TexArray&, const MaterialDielectric& mat, const Ray& r_in,
             const HitRecord& rec, vec3& attenuation, Ray& scattered) {
  attenuation = vec3(1.0f);
  float ri = rec.front_face ? (1.0 / mat.refraction_index) : mat.refraction_index;
  vec3 unit_dir = glm::normalize(r_in.direction);
  float cos_theta = glm::min(glm::dot(-unit_dir, rec.normal), 1.0f);
  float sin_theta = glm::sqrt(1.f - cos_theta * cos_theta);
  bool cannot_refract = ri * sin_theta > 1.0;
  vec3 direction;
  if (cannot_refract || SchlickReflectance(cos_theta, ri) > math::RandFloat()) {
    direction = math::Reflect(unit_dir, rec.normal);
  } else {
    direction = math::Refract(unit_dir, rec.normal, ri);
  }
  scattered = Ray{.origin = rec.point, .direction = direction, .time = r_in.time};
  return true;
}

bool Scatter(const texture::TexArray&, const MaterialLambertian& mat, const Ray& r_in,
             const HitRecord& rec, vec3& attenuation, Ray& scattered) {
  vec3 scattered_dir = rec.normal + math::RandUnitVec3();
  if (math::NearZero(scattered_dir)) {
    scattered_dir = rec.normal;
  }
  scattered = Ray{.origin = rec.point, .direction = scattered_dir, .time = r_in.time};
  attenuation = mat.albedo;
  return true;
}

bool Scatter(const texture::TexArray& tex_arr, const MaterialTexture& mat, const Ray& r_in,
             const HitRecord& rec, vec3& attenuation, Ray& scattered) {
  vec3 scattered_dir = rec.normal + math::RandUnitVec3();
  if (math::NearZero(scattered_dir)) {
    scattered_dir = rec.normal;
  }
  scattered = Ray{.origin = rec.point, .direction = scattered_dir, .time = r_in.time};
  attenuation = std::visit(
      [&rec, &tex_arr](auto&& tex) -> vec3 { return tex.Value(tex_arr, rec.uv, rec.point); },
      tex_arr[mat.tex_idx]);
  return true;
}

bool Scatter(const texture::TexArray& tex_arr, const MaterialVariant& mat, const Ray& ray,
             const HitRecord& rec, vec3& attenutation, Ray& scattered) {
  return std::visit(
      [&](const auto& material) {
        return Scatter(tex_arr, material, ray, rec, attenutation, scattered);
      },
      mat);
}

}  // namespace raytrace2::cpu
