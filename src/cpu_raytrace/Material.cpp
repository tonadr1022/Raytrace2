#include "Material.hpp"

#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Math.hpp"

namespace raytrace2::cpu {

bool MetalMaterial::Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation,
                            Ray& scattered) const {
  vec3 reflected =
      glm::normalize(math::Reflect(r_in.direction, rec.normal)) + (fuzz * math::RandUnitVec3());
  scattered = Ray{.origin = rec.point, .direction = reflected};
  attenuation = albedo;
  return true;
}

bool DielectricMaterial::Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation,
                                 Ray& scattered) const {
  attenuation = vec3(1.0f);
  float ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;
  vec3 unit_dir = glm::normalize(r_in.direction);
  float cos_theta = std::fmin(glm::dot(-unit_dir, rec.normal), 1.0);
  float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
  vec3 direction;
  if (ri * sin_theta > 1.0) {
    direction = math::Reflect(unit_dir, rec.normal);
  } else {
    direction = math::Refract(unit_dir, rec.normal, ri);
  }
  scattered = Ray{.origin = rec.point, .direction = direction};
  return true;
}

bool LambertianMaterial::Scatter(const Ray&, const HitRecord& rec, vec3& attenuation,
                                 Ray& scattered) const {
  vec3 scattered_dir = rec.normal + math::RandUnitVec3();
  if (math::NearZero(scattered_dir)) {
    scattered_dir = rec.normal;
  }
  scattered = Ray{.origin = rec.point, .direction = scattered_dir};
  attenuation = albedo;
  return true;
}

bool Scatter(const MaterialVariant& mat, const Ray& ray, const HitRecord& rec, vec3& attenutation,
             Ray& scattered) {
  return std::visit(
      [&](const auto& material) { return material.Scatter(ray, rec, attenutation, scattered); },
      mat);
}
}  // namespace raytrace2::cpu
