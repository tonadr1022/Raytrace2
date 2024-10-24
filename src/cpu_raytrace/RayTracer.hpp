#pragma once

#include <SDL_events.h>

#include "BVH.hpp"
#include "Sphere.hpp"
#include "cpu_raytrace/Camera.hpp"
#include "gl/Texture.hpp"

namespace raytrace2::cpu {

struct Scene;
class Camera;

using PixelArray = std::vector<color>;

struct RayTracer {
  void Update(const Scene& scene);

  void OnResize(glm::ivec2 dims) {
    dims_ = dims;
    camera->SetDims(dims);
    frame_idx_ = 0;

    size_t new_size = static_cast<size_t>(dims.x) * dims.y;
    pixels_.resize(new_size);
    iter_.resize(new_size);
    accumulation_data_.resize(new_size);

    int i = 0;
    for (int y = 0; y < dims.y; y++) {
      for (int x = 0; x < dims.x; x++, i++) {
        iter_[i] = {x, y, i};
      }
    }
    Reset();
  }

  [[nodiscard]] inline const gl::Texture& GetTex() const { return output_tex_; }

  [[nodiscard]] inline const PixelArray& Pixels() const { return pixels_; }

  bool OnEvent(const SDL_Event& event);
  void OnImGui();
  void Reset();

  Camera* camera{nullptr};
  size_t max_depth{50};

 private:
  gl::Texture output_tex_;
  PixelArray pixels_;
  size_t frame_idx_{0};
  std::vector<vec3> accumulation_data_;

  std::vector<glm::ivec3> iter_;
  glm::ivec2 dims_;
};

}  // namespace raytrace2::cpu
