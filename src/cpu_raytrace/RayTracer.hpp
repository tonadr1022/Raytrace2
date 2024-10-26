#pragma once

#include <SDL_events.h>

#include "BVH.hpp"
#include "Sphere.hpp"
#include "cpu_raytrace/Camera.hpp"
#include "gl/Texture.hpp"

namespace raytrace2::cpu {

struct Scene;
class Camera;

struct RayTracer {
  void Update(const Scene& scene);
  void OnResize(glm::ivec2 dims);

  [[nodiscard]] inline const gl::Texture& GetTex() const { return output_tex_; }

  [[nodiscard]] std::vector<vec3> NonConvertexPixels() const;
  [[nodiscard]] inline const PixelArray& Pixels() const { return pixels_; }

  bool OnEvent(const SDL_Event& event);
  void OnImGui();
  void Reset();

  [[nodiscard]] glm::ivec2 Dims() const { return dims_; }

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
