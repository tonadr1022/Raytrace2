#pragma once

#include "cpu_raytrace/Hittable.hpp"
namespace raytrace2::cpu {
struct ConstantMedium : public Hittable {
  ConstantMedium() = default;
  ConstantMedium(const std::shared_ptr<Hittable>& boundary, real density, uint32_t material_handle);
  bool Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const override;

  [[nodiscard]] AABB GetAABB() const override { return boundary_->GetAABB(); };

 private:
  std::shared_ptr<Hittable> boundary_;
  real neg_inv_density_;
  uint32_t material_handle_;
};

}  // namespace raytrace2::cpu
