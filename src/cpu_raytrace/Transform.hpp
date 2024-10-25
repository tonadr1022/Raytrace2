#pragma once

#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/Hittable.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtx/quaternion.hpp>
#include <utility>

#include "Defs.hpp"

namespace raytrace2::cpu {

struct Ray;
struct Transform : public Hittable {
  Transform(const std::shared_ptr<Hittable>& obj, const vec3& translation,
            const glm::quat& rotation, const vec3& scale);
  explicit Transform(const std::shared_ptr<Hittable>& obj) : obj(obj){};
  glm::mat4 model{1};
  glm::mat4 inv_model{1};
  glm::mat3 normal_mat{1};
  std::shared_ptr<Hittable> obj{nullptr};
  vec3 translation{0};
  // TODO: quaternions instead
  // glm::quat rotation = glm::angleAxis(glm::radians(45.f), vec3{0, 1, 0});
  glm::quat rotation{};
  vec3 scale{1};

  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb_; }
  [[nodiscard]] Ray Apply(const Ray& ray) const;
  [[nodiscard]] vec3 Apply(const vec3& point) const;

 private:
  void Update();
  AABB aabb_;
};

class RotateY : public Hittable {
 public:
  RotateY(const std::shared_ptr<Hittable>& object, double angle) : object(object) {
    auto radians = glm::radians(angle);
    sin_theta = std::sin(radians);
    cos_theta = std::cos(radians);
    bbox = object->GetAABB();

    vec3 min{kInfinity}, max{-kInfinity};

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
          auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
          auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
          auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

          auto newx = cos_theta * x + sin_theta * z;
          auto newz = -sin_theta * x + cos_theta * z;

          vec3 tester(newx, y, newz);

          for (int c = 0; c < 3; c++) {
            min[c] = std::fmin(min[c], tester[c]);
            max[c] = std::fmax(max[c], tester[c]);
          }
        }
      }
    }

    bbox = AABB(min, max);
  }

  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;

  [[nodiscard]] AABB GetAABB() const override { return bbox; }

 private:
  std::shared_ptr<Hittable> object;
  double sin_theta;
  double cos_theta;
  AABB bbox;
};
}  // namespace raytrace2::cpu
