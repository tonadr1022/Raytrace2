#include "Sphere.hpp"

#include "Material.hpp"
#include "cpu_raytrace/Scene.hpp"

namespace raytrace2::cpu {
bool Sphere::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  auto curr_center = center_displacement.At(r.time);
  vec3 oc = curr_center - r.origin;
  auto a = glm::dot(r.direction, r.direction);
  auto h = glm::dot(r.direction, oc);
  auto c = glm::dot(oc, oc) - radius * radius;
  auto discriminant = h * h - a * c;

  if (discriminant < 0) return false;

  auto sqrtd = std::sqrt(discriminant);

  // find the nearest root in range.
  auto root = (h - sqrtd) / a;
  if (!ray_t.Surrounds(root)) {
    root = (h + sqrtd) / a;
    if (!ray_t.Surrounds(root)) {
      return false;
    }
  }

  rec.t = root;
  rec.point = r.At(rec.t);
  rec.material = &scene.materials[material_handle];
  vec3 outward_normal = (rec.point - curr_center) / radius;
  rec.SetFaceNormal(r, outward_normal);
  rec.uv = GetUV(outward_normal);

  return true;
}

vec2 Sphere::GetUV(const vec3& p) {
  float theta = glm::acos(-p.y);
  float phi = std::atan2(-p.z, p.x) + std::numbers::pi_v<float>;
  return {phi / (2.f * std::numbers::pi_v<float>), theta / std::numbers::pi_v<float>};
}

}  // namespace raytrace2::cpu
