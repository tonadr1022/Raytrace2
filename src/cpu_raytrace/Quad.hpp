#pragma once

#include "Defs.hpp"
#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/Hittable.hpp"
#include "cpu_raytrace/HittableList.hpp"
#include "cpu_raytrace/Transform.hpp"

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

inline HittableList MakeBox(const vec3& a, const vec3& b, uint32_t material_handle) {
  HittableList list;
  vec3 min = {std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z)};
  vec3 max = {std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z)};

  vec3 dx{max.x - min.x, 0, 0};
  vec3 dy{0, max.y - min.y, 0};
  vec3 dz{0, 0, max.z - min.z};

  list.Add(std::make_shared<Quad>(vec3{min.x, min.y, max.z}, dx, dy, material_handle));   // front
  list.Add(std::make_shared<Quad>(vec3{max.x, min.y, max.z}, -dz, dy, material_handle));  // right
  list.Add(std::make_shared<Quad>(vec3{max.x, min.y, min.z}, -dx, dy, material_handle));  // back
  list.Add(std::make_shared<Quad>(vec3{min.x, min.y, min.z}, dz, dy, material_handle));   // left
  list.Add(std::make_shared<Quad>(vec3{min.x, max.y, max.z}, dx, -dz, material_handle));  // top
  list.Add(std::make_shared<Quad>(vec3{min.x, min.y, min.z}, dx, dz, material_handle));   // bottom
  return list;
}
}  // namespace raytrace2::cpu
