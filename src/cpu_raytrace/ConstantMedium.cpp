#include "ConstantMedium.hpp"

#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Interval.hpp"
#include "cpu_raytrace/Math.hpp"
#include "cpu_raytrace/Scene.hpp"

namespace raytrace2::cpu {

ConstantMedium::ConstantMedium(const std::shared_ptr<Hittable>& boundary, real density,
                               uint32_t material_handle)
    : boundary_(boundary), neg_inv_density_(-1.0 / density), material_handle_(material_handle) {}

bool ConstantMedium::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  HitRecord rec1, rec2;

  // if no intersection at all return false
  if (!boundary_->Hit(scene, r, Interval::kUniverse, rec1)) {
    return false;
  }

  // if no second instersection return false
  if (!boundary_->Hit(scene, r, Interval(rec1.t + 0.0001, kInfinity), rec2)) {
    return false;
  }

  rec1.t = std::fmax(rec1.t, ray_t.min);
  rec2.t = std::fmin(rec2.t, ray_t.max);

  // if (rec1.t < ray_t.min) rec1.t = ray_t.min;
  // if (rec2.t > ray_t.max) rec2.t = ray_t.max;

  // invalid intersection case
  if (rec1.t >= rec2.t) {
    return false;
  }

  rec1.t = std::fmax(rec1.t, 0);

  real ray_len = glm::length(r.direction);
  auto dist_inside_boundary = (rec2.t - rec1.t) * ray_len;
  auto hit_dist = neg_inv_density_ * std::log(math::RandReal());

  if (hit_dist > dist_inside_boundary) {
    return false;
  }

  rec.t = rec1.t + hit_dist / ray_len;
  rec.point = r.At(rec.t);

  // both arbitrary
  rec.normal = vec3{1, 0, 0};
  rec.front_face = true;

  rec.material = &scene.materials[material_handle_];

  return true;
}

}  // namespace raytrace2::cpu
