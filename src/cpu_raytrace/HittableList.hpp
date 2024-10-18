#pragma once

#include "Hittable.hpp"
#include "cpu_raytrace/AABB.hpp"

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

 private:
  AABB aabb_;
};

}  // namespace raytrace2::cpu
