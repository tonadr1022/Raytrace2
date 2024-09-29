#pragma once

namespace raytrace2 {

struct CameraSettings {
  float fov{90.f};
};

struct AppSettings {
  bool render_once{false};
  bool save_after_render_once{false};
  size_t num_samples{};
};
}  // namespace raytrace2
