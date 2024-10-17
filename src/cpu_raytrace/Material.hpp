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

struct alignas(16) MaterialMetal : public Material<MaterialMetal> {
  vec3 albedo;
  float fuzz{0};
  bool Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const;
};

struct alignas(16) MaterialDielectric : public Material<MaterialMetal> {
  float refraction_index;
  bool Scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered) const;
};

struct alignas(16) MaterialLambertian : public Material<MaterialLambertian> {
  vec3 albedo;
};

bool Scatter(const MaterialLambertian& mat, const Ray& r_in, const HitRecord& rec,
             vec3& attenuation, Ray& scattered);
bool Scatter(const MaterialDielectric& mat, const Ray& r_in, const HitRecord& rec,
             vec3& attenuation, Ray& scattered);
bool Scatter(const MaterialMetal& mat, const Ray& r_in, const HitRecord& rec, vec3& attenuation,
             Ray& scattered);

[[nodiscard]] extern bool Scatter(const MaterialVariant& mat, const Ray& ray, const HitRecord& rec,
                                  vec3& attenutation, Ray& scattered);

}  // namespace raytrace2::cpu
