#include "Sphere.hpp"

#include "Material.hpp"

namespace raytrace2::cpu {
bool Sphere::Hit(const Ray& r, Interval ray_t, HitRecord& rec) const {
  vec3 oc = center - r.origin;
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
  rec.material = material.get();
  rec.SetFaceNormal(r, (rec.point - center) / radius);

  return true;
}

}  // namespace raytrace2::cpu