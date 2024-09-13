#pragma once

#include <SDL_events.h>

#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/HitRecord.hpp"
#include "cpu_raytrace/Interval.hpp"
#include "cpu_raytrace/Ray.hpp"
#include "gl/Texture.hpp"

namespace raytrace2::cpu {

struct Scene;
class Camera;

using PixelArray = std::vector<glm::u8vec3>;
template <typename T>
bool HitAny(std::span<T const> hitabbles, const cpu::Ray& r, cpu::Interval ray_t,
            cpu::HitRecord& rec) {
  cpu::HitRecord temp_rec;
  bool hit_any = false;

  for (const T& hittable : hitabbles) {
    if (hittable.Hit(r, ray_t, temp_rec)) {
      hit_any = true;
      ray_t.max = temp_rec.t;
      rec = temp_rec;
    }
  }

  return hit_any;
}

struct RayTracer {
  void Update(const Scene& scene);

  void OnResize(glm::ivec2 dims) {
    pixels_.resize(static_cast<size_t>(dims.x) * dims.y);
    iter_.resize(static_cast<size_t>(dims.x) * dims.y);

    int i = 0;
    for (int y = 0; y < dims.y; y++) {
      for (int x = 0; x < dims.x; x++, i++) {
        iter_[i] = {x, y, i};
      }
    }
    camera.SetDims(dims);
  }

  [[nodiscard]] inline const gl::Texture& GetTex() const { return output_tex_; }

  [[nodiscard]] inline const PixelArray& Pixels() const { return pixels_; }

  bool OnEvent(const SDL_Event& event);

  Camera camera;

 private:
  gl::Texture output_tex_;
  PixelArray pixels_;

  std::vector<glm::ivec3> iter_;
};

}  // namespace raytrace2::cpu
