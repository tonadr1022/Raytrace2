#include "Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {
Ray Transform::Apply(const Ray& r) const {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) *
                    glm::scale(glm::mat4(1.0f), scale);
  vec3 transformed_origin = vec3(model * glm::vec4(r.origin, 1.f));
  vec3 transformed_dir = glm::rotate(rotation, r.direction);
  return Ray{.origin = transformed_origin, .direction = transformed_dir, .time = r.time};
}

}  // namespace raytrace2::cpu
