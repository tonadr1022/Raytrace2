#pragma once

#include "Defs.hpp"
#include "cpu_raytrace/Fwd.hpp"
namespace raytrace2::cpu {

struct HitRecord;
struct Ray;

enum class MaterialType { kScattering, kEmissive };

template <typename T, MaterialType Type = MaterialType::kScattering>
struct Material {
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered) const {
    if constexpr (Type == MaterialType::kScattering) {
      return static_cast<const T*>(this)->Scatter(tex_arr, r_in, rec, attenuation, scattered);
    }
    return false;
  }

  [[nodiscard]] vec3 Emit(const texture::TexArray& tex_arr, const vec2& uv, const vec3& p) const {
    if constexpr (Type == MaterialType::kEmissive) {
      return static_cast<const T*>(this)->Emit(tex_arr, uv, p);
    } else {
      return vec3{0};
    }
  }
};

struct alignas(16) MaterialMetal : public Material<MaterialMetal> {
  vec3 albedo;
  float fuzz{0};
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered) const;
};

struct alignas(16) MaterialDielectric : public Material<MaterialMetal> {
  float refraction_index;
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered) const;
};

struct alignas(16) MaterialTexture : public Material<MaterialTexture> {
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered) const;
  uint32_t tex_idx{};
};

struct DiffuseLight : public Material<DiffuseLight, MaterialType::kEmissive> {
  uint32_t tex_idx{};
  [[nodiscard]] vec3 Emit(const texture::TexArray& tex_arr, const vec2& uv, const vec3& p) const;
};

struct alignas(16) MaterialLambertian : public Material<MaterialLambertian> {
  vec3 albedo;
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered) const;
};

}  // namespace raytrace2::cpu
