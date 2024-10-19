#include "BVH.hpp"

#include "Scene.hpp"
#include "Sphere.hpp"
#include "cpu_raytrace/Math.hpp"

namespace raytrace2::cpu {

BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>>& objects, size_t start, size_t end) {
  using Comparator = bool (*)(const std::shared_ptr<Hittable>&, const std::shared_ptr<Hittable>&);
  std::array<Comparator, 3> comparators = {BoxCompareX, BoxCompareY, BoxCompareZ};
  int axis_idx = math::RandInt(0, 2);
  size_t object_span = end - start;
  if (object_span == 1) {
    left_ = objects[start];
    right_ = objects[start];
  } else if (object_span == 2) {
    left_ = objects[start];
    right_ = objects[start + 1];
  } else {
    std::sort(std::begin(objects) + start, std::begin(objects) + end, comparators[axis_idx]);
    auto mid = start + object_span / 2;
    left_ = std::make_shared<BVHNode>(objects, start, mid);
    right_ = std::make_shared<BVHNode>(objects, mid, end);
  }
  aabb_ = AABB{left_->GetAABB(), right_->GetAABB()};
}

bool BVHNode::BoxCompare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b,
                         int axis_idx) {
  return a->GetAABB().min[axis_idx] < b->GetAABB().min[axis_idx];
}

bool BVHNode::BoxCompareX(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
  return BoxCompare(a, b, 0);
}
bool BVHNode::BoxCompareY(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
  return BoxCompare(a, b, 1);
}
bool BVHNode::BoxCompareZ(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
  return BoxCompare(a, b, 2);
}

bool BVHNode::Hit(const Scene& scene, const Ray& r, Interval ray_t, HitRecord& rec) const {
  if (!aabb_.Hit(r, ray_t)) return false;
  static int i = 0;
  bool hit_left = left_->Hit(scene, r, ray_t, rec);
  bool hit_right = right_->Hit(scene, r, Interval{ray_t.min, hit_left ? rec.t : ray_t.max}, rec);
  return hit_left || hit_right;
}
}  // namespace raytrace2::cpu