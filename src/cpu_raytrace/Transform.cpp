#include "Transform.hpp"

#include "Defs.hpp"
#include "cpu_raytrace/HitRecord.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

Ray Transform::Apply(const Ray& r) const {
  vec3 transformed_origin = vec3(inv_model * glm::vec4(r.origin, 1.f));
  vec3 transformed_dir = glm::normalize(glm::mat3(inv_model) * r.direction);
  return Ray{.origin = transformed_origin, .direction = transformed_dir, .time = r.time};
}

Transform::Transform(const std::shared_ptr<Hittable>& obj, const vec3& translation,
                     const glm::quat& rotation, const vec3& scale)
    : obj(obj), translation(translation), rotation(rotation), scale(scale) {
  Update();
}

bool Transform::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  Ray transformed_r = Apply(r);
  EASSERT(obj != nullptr);
  bool hit = obj->Hit(scene, transformed_r, ray_t, rec);
  if (!hit) return false;
  // transform back to world space
  rec.point = vec3(model * glm::vec4(rec.point, 1.f));
  rec.normal = glm::normalize(normal_mat * rec.normal);
  return true;
}

void Transform::Update() {
  model = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) *
          glm::scale(glm::mat4(1.0f), scale);
  inv_model = glm::inverse(model);
  normal_mat = glm::transpose(glm::inverse(glm::mat3(model)));

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
    vec3 transformed_corner = vec3(model * glm::vec4(corner, 1.f));
    new_min.x = std::fmin(new_min.x, transformed_corner.x);
    new_min.y = std::fmin(new_min.y, transformed_corner.y);
    new_min.z = std::fmin(new_min.z, transformed_corner.z);
    new_max.x = std::fmax(new_max.x, transformed_corner.x);
    new_max.y = std::fmax(new_max.y, transformed_corner.y);
    new_max.z = std::fmax(new_max.z, transformed_corner.z);
  }

  aabb_ = AABB{new_min, new_max};
}

bool RotateY::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  // Transform the ray from world space to object space.
  auto origin = vec3((cos_theta * r.origin.x) - (sin_theta * r.origin.z), r.origin.y,
                     (sin_theta * r.origin.x) + (cos_theta * r.origin.z));

  auto direction = vec3((cos_theta * r.direction.x) - (sin_theta * r.direction.z), r.direction.y,
                        (sin_theta * r.direction.x) + (cos_theta * r.direction.z));

  Ray rotated_r(origin, direction, r.time);

  // Determine whether an intersection exists in object space (and if so, where).

  if (!object->Hit(scene, rotated_r, ray_t, rec)) return false;

  // Transform the intersection from object space back to world space.

  rec.point = vec3((cos_theta * rec.point.x) + (sin_theta * rec.point.z), rec.point.y,
                   (-sin_theta * rec.point.x) + (cos_theta * rec.point.z));

  rec.normal = vec3((cos_theta * rec.normal.x) + (sin_theta * rec.normal.z), rec.normal.y,
                    (-sin_theta * rec.normal.x) + (cos_theta * rec.normal.z));

  return true;
}
}  // namespace raytrace2::cpu
