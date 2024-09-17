#pragma once

#include "Defs.hpp"
#include "cpu_raytrace/Fwd.hpp"
namespace raytrace2::cpu {

struct HitRecord;
struct Ray;

template <typename T>
struct Material {
  bool Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const {
    return static_cast<const T*>(this)->Scatter(r_in, rec, attenuation, scattered);
  }
};

struct MetalMaterial : public Material<MetalMaterial> {
  vec3 albedo;
  bool Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const;
};

struct LambertianMaterial : public Material<LambertianMaterial> {
  vec3 albedo;
  bool Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const;
};

[[nodiscard]] extern bool Scatter(const MaterialVariant& mat, const Ray& ray, const HitRecord& rec,
                                  vec3& attenutation, Ray& scattered);

}  // namespace raytrace2::cpu
