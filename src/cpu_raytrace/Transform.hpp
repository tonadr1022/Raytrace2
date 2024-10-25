#pragma once

#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/Hittable.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Defs.hpp"

namespace raytrace2::cpu {

struct Ray;
struct Transform : public Hittable {
  Transform(const std::shared_ptr<Hittable>& obj, const vec3& translation, const quat& rotation,
            const vec3& scale);
  mat4 model;
  mat4 inv_model;
  mat3 normal_mat;
  std::shared_ptr<Hittable> obj;
  vec3 translation{0};
  quat rotation{1, 0, 0, 0};
  vec3 scale{1};

  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb_; }
  [[nodiscard]] Ray WorldToModel(const Ray& ray) const;
  [[nodiscard]] vec3 Apply(const vec3& point) const;

 private:
  void Init();
  AABB aabb_;
};

}  // namespace raytrace2::cpu
