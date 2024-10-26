#include "Transform.hpp"

#include "Defs.hpp"
#include "cpu_raytrace/HitRecord.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {

Ray TransformedHittable::WorldToModel(const Ray& r) const {
  // transform origin from world to model space
  vec3 transformed_origin = vec3(inv_model * vec4(r.origin, 1));
  // transform direction to model space without translation component
  vec3 transformed_dir = glm::normalize(mat3(inv_model) * r.direction);
  return Ray{.origin = transformed_origin, .direction = transformed_dir, .time = r.time};
  // return Ray{.origin = transformed_origin, .direction = r.direction, .time = r.time};
}
bool isIdentityMatrix(const glm::mat4& matrix, float epsilon = 1e-6f) {
  // Identity matrix for reference
  auto identity = glm::mat4(1.0f);

  // Check each element for near-equality within the tolerance epsilon
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (std::abs(matrix[i][j] - identity[i][j]) > epsilon) {
        return false;
      }
    }
  }
  return true;
}

void TransformedHittable::Init() {
  inv_model = glm::inverse(model);
  normal_mat = glm::transpose(glm::inverse(model));

  EASSERT(obj != nullptr);

  // transform the existing aabb with model matrix
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
TransformedHittable::TransformedHittable(const std::shared_ptr<Hittable>& obj, mat4 transform)
    : model(transform), obj(obj) {
  Init();
}
TransformedHittable::TransformedHittable(const std::shared_ptr<Hittable>& obj,
                                         const Transform& transform)
    : model(transform.model), obj(obj) {
  Init();
}

bool TransformedHittable::Hit(const Scene& scene, const Ray& r, Interval ray_t,
                              HitRecord& rec) const {
  // transform ray to model space, hit object in model space, transform hit point and normal back to
  // world space

  Ray model_space_ray = WorldToModel(r);
  EASSERT(obj != nullptr);
  bool hit = obj->Hit(scene, model_space_ray, ray_t, rec);
  if (!hit) return false;
  // transform back to world space
  rec.point = vec3(model * vec4(rec.point, 1.f));
  rec.normal = glm::normalize(normal_mat * rec.normal);
  return true;
}

}  // namespace raytrace2::cpu
