#include "BVH.hpp"

#include "Defs.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "cpu_raytrace/Math.hpp"

namespace raytrace2::cpu {

BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable>>& objects, size_t start, size_t end) {
  using Comparator = bool (*)(const std::shared_ptr<Hittable>&, const std::shared_ptr<Hittable>&);
  std::array<Comparator, 3> comparators = {BoxCompareX, BoxCompareY, BoxCompareZ};
  size_t object_span = end - start;
  aabb_.min = glm::vec3{kInfinity};
  aabb_.max = glm::vec3{-kInfinity};
  for (size_t object_idx = start; object_idx < end; object_idx++) {
    aabb_ = AABB{aabb_, objects[object_idx]->GetAABB()};
  }
  if (object_span == 1) {
    left_ = objects[start];
    right_ = objects[start];
  } else if (object_span == 2) {
    left_ = objects[start];
    right_ = objects[start + 1];
  } else {
    std::sort(std::begin(objects) + start, std::begin(objects) + end,
              comparators[aabb_.LongestAxis()]);
    auto mid = start + object_span / 2;
    left_ = std::make_shared<BVHNode>(objects, start, mid);
    right_ = std::make_shared<BVHNode>(objects, mid, end);
  }
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
  bool hit_left = left_->Hit(scene, r, ray_t, rec);
  bool hit_right = right_->Hit(scene, r, Interval{ray_t.min, hit_left ? rec.t : ray_t.max}, rec);
  return hit_left || hit_right;
}
}  // namespace raytrace2::cpu
