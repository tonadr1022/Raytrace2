#include "RayTracer.hpp"

#include <execution>

#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/Scene.hpp"

namespace raytrace2::cpu {

namespace {

color ToColor(const vec3& col) {
  return color{floor(col.x * 255.999), floor(col.y * 255.999), floor(col.z * 255.999)};
}

vec3 UnitSphereVecToColor(const vec3& normal) {
  return vec3{normal.x + 1, normal.y + 1, normal.z + 1} * 0.5;
}

color RayColor(const cpu::Ray& r, const cpu::Scene& scene) {
  cpu::HitRecord rec;
  bool hit_any_sphere =
      HitAny(std::span<cpu::Sphere const>(scene.spheres), r, cpu::Interval{0, kInfinity}, rec);
  if (hit_any_sphere) {
    return ToColor(UnitSphereVecToColor(rec.normal));
  }

  vec3 unit_dir = glm::normalize(r.direction);
  auto a = 0.5 * (unit_dir.y + 1.0);
  vec3 col = (1.0 - a) * vec3{1, 1, 1} + a * vec3{0.5, 0.7, 1.0};
  return ToColor(col);
}

}  // namespace

void RayTracer::Update(const Scene& scene) {
  auto per_pixel = [this, &scene](glm::ivec3& idx) {
    pixels_[idx.z] = RayColor(camera.GetRay(idx), scene);
  };
  std::for_each(std::execution::par, iter_.begin(), iter_.end(), per_pixel);
}

bool RayTracer::OnEvent(const SDL_Event& event) {
  if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
    OnResize(glm::ivec2{event.window.data1, event.window.data2});
    return true;
  }
  return false;
}
}  // namespace raytrace2::cpu
