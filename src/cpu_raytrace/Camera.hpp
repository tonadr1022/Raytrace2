#pragma once

#include "Defs.hpp"
#include "Math.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {
class Camera {
 public:
  void OnChange() {
    float focal_length = 1.0;
    float viewport_height = 2.0;
    float viewport_width = viewport_height * (static_cast<float>(dims_.x) / dims_.y);

    vec3 viewport_length_u = vec3(viewport_width, 0, 0);
    vec3 viewport_length_v = vec3(0, viewport_height, 0);

    pixel_delta_u_ = viewport_length_u / static_cast<float>(dims_.x);
    pixel_delta_v_ = viewport_length_v / static_cast<float>(dims_.y);

    viewport_upper_left_ =
        center_ - vec3(0, 0, focal_length) - viewport_length_u / 2.f - viewport_length_v / 2.f;
    pixel00_loc_ = viewport_upper_left_ + 0.5f * (pixel_delta_u_ + pixel_delta_v_);
  }

  [[nodiscard]] inline Ray GetRay(int x, int y) const {
    auto pixel_center = pixel00_loc_ + (static_cast<float>(x) * pixel_delta_u_) +
                        (static_cast<float>(y) * pixel_delta_v_);
    auto pixel_sample_square = [this]() -> vec3 {
      float px = -0.5 + math::RandFloat0To1();
      float py = -0.5 + math::RandFloat0To1();
      return {px * pixel_delta_u_.x, py * pixel_delta_v_.y, 0.0};
    };

    pixel_center += pixel_sample_square();

    return {.origin = center_, .direction = pixel_center - center_};
  }

  inline void SetCenter(const glm::vec3& center) {
    center_ = center;
    OnChange();
  }

  inline void SetDims(const glm::ivec2& dims) {
    dims_ = dims;
    OnChange();
  }

  [[nodiscard]] inline const glm::ivec2& Dims() const { return dims_; }

 private:
  vec3 center_;
  vec3 viewport_upper_left_;
  vec3 pixel00_loc_;
  vec3 pixel_delta_u_, pixel_delta_v_;
  glm::ivec2 dims_;
};
}  // namespace raytrace2::cpu
