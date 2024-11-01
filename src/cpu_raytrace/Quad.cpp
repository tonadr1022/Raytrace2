#include "Quad.hpp"

#include "Defs.hpp"
#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Interval.hpp"
#include "cpu_raytrace/Math.hpp"
#include "cpu_raytrace/Scene.hpp"

namespace raytrace2::cpu {
bool IsInterior(real a, real b, HitRecord& rec) {
  Interval unit_interval{0, 1};

  // given the hit point in the plane coordinates, not interior if it's outside
  // the primitive, otherwise set the hit uv coords and return true
  if (!unit_interval.Contains(a) || !unit_interval.Contains(b)) return false;

  rec.uv = {a, b};
  return true;
}

bool Quad::Hit(const Ray& r, Interval ray_t, HitRecord& rec) const {
  real n_dot_raydir = glm::dot(normal, r.direction);

  // no hit if ray parallel to plane
  if (std::fabs(n_dot_raydir) < 1e-8) return false;

  // no hit if hit point t outside ray interval
  real t = (d - glm::dot(normal, r.origin)) / n_dot_raydir;
  if (!ray_t.Contains(t)) return false;

  // determine if hit point lies within planer shape using its plane coords
  vec3 intersection_pt = r.At(t);
  vec3 planar_hitpt_vector = intersection_pt - q;
  real alpha = glm::dot(w, glm::cross(planar_hitpt_vector, v));
  real beta = glm::dot(w, glm::cross(u, planar_hitpt_vector));

  if (!IsInterior(alpha, beta, rec)) return false;

  rec.t = t;
  rec.point = intersection_pt;
  rec.SetFaceNormal(r, normal);

  return true;
}

bool Quad::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  bool hit = Hit(r, ray_t, rec);
  if (hit) {
    rec.material = &scene.materials[material_handle];
  }
  return hit;
}

[[nodiscard]] real Quad::PDFValue([[maybe_unused]] const vec3& origin,
                                  [[maybe_unused]] const vec3& dir) const {
  HitRecord rec;
  if (!Hit(Ray{origin, dir, 0}, Interval(0.001, kInfinity), rec)) return 0;

  real dist_squared = rec.t * rec.t * glm::dot(dir, dir);
  real cosine = std::fabs(glm::dot(dir, rec.normal) / glm::length(dir));
  return dist_squared / (cosine * area);
}

vec3 Quad::Random(const vec3& origin) const {
  auto p = q + (math::RandReal() * u) + (math::RandReal() * v);
  return p - origin;
}

}  // namespace raytrace2::cpu
