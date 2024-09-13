#pragma once

#include "cpu_raytrace/Sphere.hpp"

namespace raytrace2::cpu {

struct Scene {
  std::vector<cpu::Sphere> spheres;
};

}  // namespace raytrace2::cpu
