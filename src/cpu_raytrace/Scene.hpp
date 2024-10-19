#pragma once

#include "cpu_raytrace/BVH.hpp"
#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/HittableList.hpp"
#include "cpu_raytrace/Material.hpp"
#include "cpu_raytrace/Sphere.hpp"

namespace raytrace2::cpu {

struct BVHNode;

struct Scene {
  HittableList hittable_list;
  std::vector<MaterialVariant> materials;
  Camera cam;
  std::string cam_name;
};

}  // namespace raytrace2::cpu
