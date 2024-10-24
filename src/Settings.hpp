#pragma once

namespace raytrace2 {

struct AppSettings {
  bool render_once{true};
  bool save_after_render_once{false};
  size_t num_samples{};
  size_t max_depth{50};
};
}  // namespace raytrace2
