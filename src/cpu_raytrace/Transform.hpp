#pragma once

#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/Hittable.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Defs.hpp"

namespace raytrace2::cpu {

struct Ray;

struct Transform {
  void Apply(const vec3& translation, const quat& rotation, const vec3& scale);
  mat4 model{1};
};

struct TransformedHittable : public Hittable {
  TransformedHittable(const std::shared_ptr<Hittable>& obj, const Transform& transform);
  TransformedHittable(const std::shared_ptr<Hittable>& obj, mat4 transform);
  mat4 model;
  mat4 inv_model;
  mat3 normal_mat;
  std::shared_ptr<Hittable> obj;

  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb_; }
  [[nodiscard]] Ray WorldToModel(const Ray& ray) const;
  [[nodiscard]] vec3 Apply(const vec3& point) const;

 private:
  void Init();
  AABB aabb_;
};

}  // namespace raytrace2::cpu
