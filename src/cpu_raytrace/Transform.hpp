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
  Transform(const std::shared_ptr<Hittable>& obj, const vec3& translation,
            const glm::quat& rotation, const vec3& scale);
  explicit Transform(const std::shared_ptr<Hittable>& obj) : obj(obj){};
  glm::mat4 model{1};
  glm::mat4 inv_model{1};
  glm::mat3 transpose_normal_mat{1};
  std::shared_ptr<Hittable> obj{nullptr};
  vec3 translation{0};
  // TODO: quaternions instead
  // glm::quat rotation = glm::angleAxis(glm::radians(45.f), vec3{0, 1, 0});
  glm::quat rotation{};
  vec3 scale{1};
  bool valid{false};

  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb_; }
  [[nodiscard]] Ray Apply(const Ray& ray) const;
  [[nodiscard]] vec3 Apply(const vec3& point) const;

 private:
  void Update();
  AABB aabb_;
};
}  // namespace raytrace2::cpu
