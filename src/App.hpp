#pragma once

#include <SDL_events.h>
namespace raytrace2 {

class App {
 public:
  void Run();
  void OnEvent(SDL_Event& event);
};

}  // namespace raytrace2
