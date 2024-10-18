#include "HittableList.hpp"

#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Scene.hpp"

namespace raytrace2::cpu {

bool HittableList::Hit(const Scene& scene, const cpu::Ray& r, cpu::Interval ray_t,
                       cpu::HitRecord& rec) const {
  cpu::HitRecord temp_rec;
  bool hit_any = false;

  for (const auto& hittable : objects) {
    bool hit = hittable->Hit(scene, r, ray_t, temp_rec);
    if (hit) {
      hit_any = true;
      ray_t.max = temp_rec.t;
      rec = temp_rec;
    }
  }
  return hit_any;
}

}  // namespace raytrace2::cpu
