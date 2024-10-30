#pragma once

#include <SDL_events.h>

#include "Settings.hpp"
#include "Window.hpp"
#include "cpu_raytrace/RayTracer.hpp"

namespace raytrace2 {

class App {
 public:
  void Run(int argc, char* argv[]);
  void OnEvent(SDL_Event& event);

 private:
  void OnResize(glm::ivec2 dims);
  cpu::RayTracer cpu_tracer_;
  std::unique_ptr<Window> window_{nullptr};
  AppSettings settings_;
  bool imgui_enabled_{true};
};

}  // namespace raytrace2
