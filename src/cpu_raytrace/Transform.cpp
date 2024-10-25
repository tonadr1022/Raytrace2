#include "Transform.hpp"

#include "Defs.hpp"
#include "cpu_raytrace/HitRecord.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Math.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

Ray Transform::Apply(const Ray& r) const {
  vec3 transformed_origin = vec3(inv_model * vec4(r.origin, 1));
  // vec3 transformed_dir = glm::normalize(mat3(inv_model) * r.direction);
  // vec3 transformed_dir = vec3(glm::normalize(glm::inverse(mat3(model)) * vec4(r.direction,
  // 0.0)));
  vec3 transformed_dir = glm::normalize(vec3(inv_model * vec4(glm::normalize(r.direction), 0.0)));
  // Debugging output to check if direction is unchanged after translation
  // auto original = glm::normalize(r.direction);
  // if (std::fabs(original.x - transformed_dir.x) > 0.0001 ||
  //     std::fabs(original.y - transformed_dir.y) > 0.0001 ||
  //     std::fabs(original.z - transformed_dir.z) > 0.0001) {
  //   std::cout << "error\n";
  //   std::cout << original.x << ' ' << original.y << ' ' << original.z << '\n';
  //   std::cout << transformed_dir.x << ' ' << transformed_dir.y << ' ' << transformed_dir.z <<
  //   '\n'; exit(1);
  // }
  return Ray{.origin = transformed_origin, .direction = transformed_dir, .time = r.time};
  // return Ray{.origin = transformed_origin, .direction = r.direction, .time = r.time};
}

Transform::Transform(const std::shared_ptr<Hittable>& obj, const vec3& translation,
                     const quat& rotation, const vec3& scale)
    : obj(obj), translation(translation), rotation(rotation), scale(scale) {
  Update();
}

bool Transform::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  Ray transformed_r = Apply(r);
  EASSERT(obj != nullptr);
  bool hit = obj->Hit(scene, transformed_r, ray_t, rec);
  if (!hit) return false;
  // transform back to world space
  rec.point = vec3(model * vec4(rec.point, 1.f));
  rec.normal = glm::normalize(normal_mat * rec.normal);
  return true;
}

void Transform::Update() {
  // model = glm::translate(mat4(1), translation);
  model = glm::translate(mat4(1), translation) * glm::toMat4(rotation) * glm::scale(mat4(1), scale);
  inv_model = glm::inverse(model);
  normal_mat = glm::transpose(glm::inverse(model));

  EASSERT(obj != nullptr);
  AABB existing_aabb = obj->GetAABB();
  auto min = existing_aabb.GetMin();
  auto max = existing_aabb.GetMax();
  vec3 corners[8] = {
      vec3(min.x, min.y, min.z), vec3(max.x, min.y, min.z), vec3(min.x, max.y, min.z),
      vec3(max.x, max.y, min.z), vec3(min.x, min.y, max.z), vec3(max.x, min.y, max.z),
      vec3(min.x, max.y, max.z), vec3(max.x, max.y, max.z),
  };

  vec3 new_min = vec3(kInfinity);
  vec3 new_max = vec3(-kInfinity);

  for (const vec3& corner : corners) {
    vec3 transformed_corner = vec3(model * vec4(corner, 1.f));
    new_min.x = std::fmin(new_min.x, transformed_corner.x);
    new_min.y = std::fmin(new_min.y, transformed_corner.y);
    new_min.z = std::fmin(new_min.z, transformed_corner.z);
    new_max.x = std::fmax(new_max.x, transformed_corner.x);
    new_max.y = std::fmax(new_max.y, transformed_corner.y);
    new_max.z = std::fmax(new_max.z, transformed_corner.z);
  }

  aabb_ = AABB{new_min, new_max};
}

bool Translate::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  // Move the ray backwards by the offset
  Ray offset_r(r.origin - offset_, r.direction, r.time);

  // Determine whether an intersection exists along the offset ray (and if so, where)
  if (!object_->Hit(scene, offset_r, ray_t, rec)) return false;

  // Move the intersection point forwards by the offset
  rec.point += offset_;

  return true;
}

bool RotateY::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  // Transform the ray from world space to object space.
  auto origin = vec3((cos_theta_ * r.origin.x) - (sin_theta_ * r.origin.z), r.origin.y,
                     (sin_theta_ * r.origin.x) + (cos_theta_ * r.origin.z));

  auto direction = vec3((cos_theta_ * r.direction.x) - (sin_theta_ * r.direction.z), r.direction.y,
                        (sin_theta_ * r.direction.x) + (cos_theta_ * r.direction.z));

  Ray rotated_r(origin, direction, r.time);

  // Determine whether an intersection exists in object space (and if so, where).

  if (!object_->Hit(scene, rotated_r, ray_t, rec)) return false;

  // Transform the intersection from object space back to world space.

  rec.point = vec3((cos_theta_ * rec.point.x) + (sin_theta_ * rec.point.z), rec.point.y,
                   (-sin_theta_ * rec.point.x) + (cos_theta_ * rec.point.z));

  rec.normal = vec3((cos_theta_ * rec.normal.x) + (sin_theta_ * rec.normal.z), rec.normal.y,
                    (-sin_theta_ * rec.normal.x) + (cos_theta_ * rec.normal.z));

  return true;
}
}  // namespace raytrace2::cpu
