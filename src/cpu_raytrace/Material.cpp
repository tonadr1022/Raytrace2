#include "Material.hpp"

#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Math.hpp"

namespace raytrace2::cpu {

bool MetalMaterial::Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation,
                            Ray& scattered) const {
  static int i = 0;
  if (i++ < 10) {
    std::cout << "metal: " << albedo.r << ' ' << albedo.g << ' ' << albedo.b << '\n';
  }
  vec3 reflected = math::Reflect(r_in.direction, rec.normal);
  scattered = Ray{.origin = rec.point, .direction = reflected};
  attenuation = albedo;
  return true;
}

bool LambertianMaterial::Scatter(const Ray&, const HitRecord& rec, vec3& attenuation,
                                 Ray& scattered) const {
  static int i = 0;
  if (i++ < 10) {
    std::cout << albedo.r << ' ' << albedo.g << ' ' << albedo.b << '\n';
  }
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
