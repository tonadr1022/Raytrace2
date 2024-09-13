#pragma once

#include "Defs.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {
class Camera {
 public:
  void OnChange() {
    double focal_length = 1.0;
    double viewport_height = 2.0;
    double viewport_width = viewport_height * (static_cast<double>(dims_.x) / dims_.y);

    vec3 viewport_length_u = vec3(viewport_width, 0, 0);
    vec3 viewport_length_v = vec3(0, viewport_height, 0);

    pixel_delta_u_ = viewport_length_u / static_cast<double>(dims_.x);
    pixel_delta_v_ = viewport_length_v / static_cast<double>(dims_.y);

    viewport_upper_left_ =
        center_ - vec3(0, 0, focal_length) - viewport_length_u / 2. - viewport_length_v / 2.;
    pixel00_loc_ = viewport_upper_left_ + 0.5 * (pixel_delta_u_ + pixel_delta_v_);
  }

  [[nodiscard]] inline Ray GetRay(glm::ivec2 idx) const {
    auto pixel_center = pixel00_loc_ + (static_cast<double>(idx.x) * pixel_delta_u_) +
                        (static_cast<double>(idx.y) * pixel_delta_v_);
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
