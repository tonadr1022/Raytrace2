#pragma once

namespace raytrace2 {

struct AppSettings {
  bool render_once;
  bool save_after_render_once;
  size_t num_samples;
  size_t max_depth;
  bool render_window;
};
}  // namespace raytrace2
