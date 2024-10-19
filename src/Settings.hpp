#pragma once

namespace raytrace2 {

struct AppSettings {
  bool render_once{false};
  bool save_after_render_once{false};
  size_t num_samples{};
};
}  // namespace raytrace2
