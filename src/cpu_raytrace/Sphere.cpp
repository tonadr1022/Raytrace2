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
  rec.SetFaceNormal(r, (rec.point - curr_center) / radius);

  return true;
}

}  // namespace raytrace2::cpu
