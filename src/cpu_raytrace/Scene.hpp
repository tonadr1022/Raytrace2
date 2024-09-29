#pragma once

#include "cpu_raytrace/Material.hpp"
#include "cpu_raytrace/Sphere.hpp"

namespace raytrace2::cpu {

struct Scene {
  std::vector<cpu::Sphere> spheres;
  std::vector<MaterialVariant> materials;
};

}  // namespace raytrace2::cpu
