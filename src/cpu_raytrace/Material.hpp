#pragma once

#include "Defs.hpp"
#include "cpu_raytrace/Fwd.hpp"
namespace raytrace2::cpu {

struct HitRecord;
struct Ray;

enum class MaterialType { kScattering, kEmissive };

template <typename T, MaterialType Type = MaterialType::kScattering, bool HasPDF = false>
struct Material {
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered, real& pdf) const {
    if constexpr (Type == MaterialType::kScattering) {
      return static_cast<const T*>(this)->Scatter(tex_arr, r_in, rec, attenuation, scattered, pdf);
    }
    return false;
  }

  [[nodiscard]] real ScatteringPDF(const Ray& r_in, const HitRecord& rec,
                                   const Ray& scattered) const {
    if constexpr (HasPDF) {
      return static_cast<const T*>(this)->ScatteringPDF2(r_in, rec, scattered);
    }
    return 0;
  }

  [[nodiscard]] vec3 Emit(const texture::TexArray& tex_arr, const HitRecord& rec, const vec2& uv,
                          const vec3& p) const {
    if constexpr (Type == MaterialType::kEmissive) {
      return static_cast<const T*>(this)->Emit(tex_arr, rec, uv, p);
    } else {
      return vec3{0};
    }
  }
};

struct EmptyMaterial : public Material<EmptyMaterial, MaterialType::kScattering, false> {
  bool Scatter(const texture::TexArray&, const Ray&, const HitRecord&, vec3&, Ray&, real&) const {
    return false;
  }
};

struct MaterialMetal : public Material<MaterialMetal> {
  vec3 albedo;
  real fuzz{0};
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered, real& pdf) const;
};

struct MaterialDielectric : public Material<MaterialDielectric> {
  real refraction_index;
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered, real& pdf) const;
};

struct MaterialTexture : public Material<MaterialTexture> {
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered, real& pdf) const;
  uint32_t tex_idx{};
};

struct DiffuseLight : public Material<DiffuseLight, MaterialType::kEmissive> {
  uint32_t tex_idx{};
  bool double_sided{false};
  [[nodiscard]] vec3 Emit(const texture::TexArray& tex_arr, const HitRecord& rec, const vec2& uv,
                          const vec3& p) const;
};

struct MaterialLambertian : public Material<MaterialLambertian, MaterialType::kScattering, true> {
  vec3 albedo;
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered, real& pdf) const;
  [[nodiscard]] real ScatteringPDF(const Ray& r_in, const HitRecord& rec,
                                   const Ray& scattered) const;
};

struct MaterialIsotropic : public Material<MaterialIsotropic, MaterialType::kScattering, true> {
  uint32_t tex_idx{};
  bool Scatter(const texture::TexArray& tex_arr, const Ray& r_in, const HitRecord& rec,
               vec3& attenuation, Ray& scattered, real& pdf) const;
  [[nodiscard]] real ScatteringPDF(const Ray& r_in, const HitRecord& rec,
                                   const Ray& scattered) const;
};

}  // namespace raytrace2::cpu
