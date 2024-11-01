#pragma once

#include "Hittable.hpp"
#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/Math.hpp"

namespace raytrace2::cpu {

struct HittableList : public Hittable {
  HittableList() = default;
  explicit HittableList(const std::shared_ptr<Hittable>& obj) { Add(obj); }
  std::vector<std::shared_ptr<Hittable>> objects;

  void Add(const std::shared_ptr<Hittable>& object) {
    objects.emplace_back(object);
    aabb_ = AABB{aabb_, object->GetAABB()};
  }
  bool Hit(const Scene& scene, const cpu::Ray& r, cpu::Interval ray_t,
           cpu::HitRecord& rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb_; }

  [[nodiscard]] real PDFValue(const vec3& origin, const vec3& dir) const override {
    if (objects.empty()) return 0;
    real sum = 0.0;
    for (const auto& obj : objects) {
      sum += obj->PDFValue(origin, dir);
    }
    return sum / objects.size();
  }

  [[nodiscard]] vec3 Random([[maybe_unused]] const vec3& origin) const override {
    return objects[math::RandInt(0, static_cast<int>(objects.size()) - 1)]->Random(origin);
  }

 private:
  AABB aabb_;
};

}  // namespace raytrace2::cpu
