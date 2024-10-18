#pragma once

#include "cpu_raytrace/AABB.hpp"
#include "cpu_raytrace/HittableList.hpp"

namespace raytrace2::cpu {

struct Scene;
struct HitRecord;

struct BVHNode : public Hittable {
 public:
  explicit BVHNode(HittableList list) : BVHNode(list.objects, 0, list.objects.size()) {}
  BVHNode(std::vector<std::shared_ptr<Hittable>> &objects, size_t start, size_t end);
  bool Hit(const Scene &scene, const Ray &r, Interval ray_t, HitRecord &rec) const override;
  [[nodiscard]] AABB GetAABB() const override { return aabb_; }

 private:
  std::shared_ptr<Hittable> left_;
  std::shared_ptr<Hittable> right_;
  AABB aabb_;

  static bool BoxCompare(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b,
                         int axis_idx);
  static bool BoxCompareX(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b);
  static bool BoxCompareY(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b);
  static bool BoxCompareZ(const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b);
};

}  // namespace raytrace2::cpu
