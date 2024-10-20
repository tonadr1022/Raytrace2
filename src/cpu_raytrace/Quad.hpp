#pragma once

#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/Hittable.hpp"
namespace raytrace2::cpu {
struct Scene;
struct Quad : public Hittable {
  Quad(const vec3& q, const vec3& u, const vec3& v, uint32_t material_handle)
      : q(q), u(u), v(v), material_handle(material_handle) {
    auto n = glm::cross(u, v);
    normal = glm::normalize(n);
    d = glm::dot(normal, q);
    w = n / glm::dot(n, n);
    SetBoundingBox();
  }

  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb; };

  void SetBoundingBox() { aabb = AABB{AABB{q, q + u + v}, AABB{q + u, q + v}}; }

  AABB aabb;
  vec3 q, u, v, w, normal;
  float d;
  uint32_t material_handle;
};
}  // namespace raytrace2::cpu
