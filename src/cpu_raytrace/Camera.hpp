#pragma once

#include <glm/trigonometric.hpp>

#include "Defs.hpp"
#include "Math.hpp"
#include "cpu_raytrace/Ray.hpp"

namespace raytrace2::cpu {
class Camera {
 public:
  Camera() = default;
  Camera(const glm::vec3& center, const glm::vec3& lookat, const glm::vec3& view_up)
      : center_(center), lookat_(lookat), view_up_(view_up) {}

  void OnChange() {
    real theta = glm::radians(vfov_);
    real h = glm::tan(theta / 2);
    vec3 u, v, w;
    // real focal_length = glm::length(center_ - lookat_);
    w = glm::normalize(center_ - lookat_);
    u = glm::normalize(glm::cross(view_up_, w));
    v = glm::cross(w, u);

    real viewport_height = 2.0 * h * focus_dist_;
    real viewport_width = viewport_height * (static_cast<real>(dims_.x) / dims_.y);

    vec3 viewport_length_u = viewport_width * u;
    vec3 viewport_length_v = viewport_height * v;

    pixel_delta_u_ = viewport_length_u / static_cast<real>(dims_.x);
    pixel_delta_v_ = viewport_length_v / static_cast<real>(dims_.y);

    viewport_upper_left_ = center_ - vec3(w * focus_dist_) -
                           viewport_length_u / static_cast<real>(2.0) -
                           viewport_length_v / static_cast<real>(2.0);
    pixel00_loc_ =
        viewport_upper_left_ + static_cast<real>(0.5) * (pixel_delta_u_ + pixel_delta_v_);

    auto defocus_radius = focus_dist_ * glm::tan(glm::radians(defocus_angle_ / 2));
    defocus_disk_u_ = u * defocus_radius;
    defocus_disk_v_ = v * defocus_radius;
  }

  [[nodiscard]] inline Ray GetRay(int x, int y) const {
    auto pixel_center = pixel00_loc_ + (static_cast<real>(x) * pixel_delta_u_) +
                        (static_cast<real>(y) * pixel_delta_v_);
    auto pixel_sample_square = [this]() -> vec3 {
      real px = -0.5 + math::RandReal();
      real py = -0.5 + math::RandReal();
      return {px * pixel_delta_u_.x, py * pixel_delta_v_.y, 0.0};
    };

    pixel_center += pixel_sample_square();
    vec3 center = (defocus_angle_ <= 0) ? center_ : DefocusDiskSample();
    // assuming time starts at 0 and ends at 1, randomly sample a time between
    real ray_time = math::RandReal();
    // return Ray{
    //     .origin = center, .direction = glm::normalize(pixel_center - center), .time = ray_time};
    return Ray{.origin = center, .direction = pixel_center - center, .time = ray_time};
  }

  inline void SetCenter(const vec3& center) {
    center_ = center;
    OnChange();
  }

  inline void SetViewUp(const vec3& view_up) {
    view_up_ = view_up;
    OnChange();
  }

  inline void SetLookAt(const vec3& lookat) {
    lookat_ = lookat;
    OnChange();
  }

  inline void SetFOV(real fov) {
    vfov_ = fov;
    OnChange();
  }

  inline void SetDims(const glm::ivec2& dims) {
    dims_ = dims;
    OnChange();
  }

  inline void SetDefocusAngle(real angle) {
    defocus_angle_ = angle;
    OnChange();
  }

  inline void SetFocusDistance(real focus_distance) {
    focus_dist_ = focus_distance;
    OnChange();
  }

  [[nodiscard]] inline real GetFOV() const { return vfov_; }
  [[nodiscard]] inline const glm::ivec2& GetDims() const { return dims_; }

  vec3 center_{0, 0, 0};
  vec3 lookat_{0, 0, -1};
  vec3 view_up_{0, 1, 0};
  vec3 viewport_upper_left_;
  vec3 pixel00_loc_;
  vec3 pixel_delta_u_, pixel_delta_v_;
  vec3 defocus_disk_u_;
  vec3 defocus_disk_v_;
  real defocus_angle_{0};
  real focus_dist_{10};
  real vfov_{90.f};
  glm::ivec2 dims_;

 private:
  [[nodiscard]] vec3 DefocusDiskSample() const {
    auto p = math::RandInUnitDisk();
    return center_ + (p[0] * defocus_disk_u_) + (p[1] * defocus_disk_v_);
  }
};
}  // namespace raytrace2::cpu
