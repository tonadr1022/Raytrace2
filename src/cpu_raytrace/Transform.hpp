#pragma once

#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/Hittable.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Defs.hpp"

namespace raytrace2::cpu {

struct Ray;
struct Transform : public Hittable {
  Transform(const std::shared_ptr<Hittable>& obj, const vec3& translation, const quat& rotation,
            const vec3& scale);
  mat4 model{1};
  mat4 inv_model{1};
  mat3 normal_mat{1};
  std::shared_ptr<Hittable> obj{nullptr};
  vec3 translation{0};
  // TODO: quaternions instead
  // glm::quat rotation = glm::angleAxis(glm::radians(45.f), vec3{0, 1, 0});
  quat rotation{1, 0, 0, 0};
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
  RotateY(const std::shared_ptr<Hittable>& object, double angle) : object_(object) {
    auto radians = glm::radians(angle);
    sin_theta_ = std::sin(radians);
    cos_theta_ = std::cos(radians);
    bbox_ = object->GetAABB();

    vec3 min{kInfinity}, max{-kInfinity};

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
          auto x = i * bbox_.x.max + (1 - i) * bbox_.x.min;
          auto y = j * bbox_.y.max + (1 - j) * bbox_.y.min;
          auto z = k * bbox_.z.max + (1 - k) * bbox_.z.min;

          auto newx = cos_theta_ * x + sin_theta_ * z;
          auto newz = -sin_theta_ * x + cos_theta_ * z;

          vec3 tester(newx, y, newz);

          for (int c = 0; c < 3; c++) {
            min[c] = std::fmin(min[c], tester[c]);
            max[c] = std::fmax(max[c], tester[c]);
          }
        }
      }
    }

    bbox_ = AABB(min, max);
  }

  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;

  [[nodiscard]] AABB GetAABB() const override { return bbox_; }

 private:
  std::shared_ptr<Hittable> object_;
  double sin_theta_;
  double cos_theta_;
  AABB bbox_;
};
class Translate : public Hittable {
 public:
  Translate(const std::shared_ptr<Hittable>& object, const vec3& offset)
      : object_(object), offset_(offset) {
    bbox_ = object->GetAABB() + offset;
  }
  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;

  [[nodiscard]] AABB GetAABB() const override { return bbox_; }

 private:
  std::shared_ptr<Hittable> object_;
  AABB bbox_;
  vec3 offset_;
};
}  // namespace raytrace2::cpu
