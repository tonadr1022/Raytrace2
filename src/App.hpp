#pragma once

#include <SDL_events.h>

#include "cpu_raytrace/RayTracer.hpp"
namespace raytrace2 {

class App {
 public:
  void Run();
  void OnEvent(SDL_Event& event);

 private:
  void OnResize(glm::ivec2 dims);
  cpu::RayTracer cpu_tracer_;
};

}  // namespace raytrace2
