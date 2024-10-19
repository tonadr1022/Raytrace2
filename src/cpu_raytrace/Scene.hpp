#pragma once

#include "Fwd.hpp"
#include "Material.hpp"
#include "cpu_raytrace/BVH.hpp"
#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/HittableList.hpp"
#include "cpu_raytrace/Sphere.hpp"
#include "cpu_raytrace/Texture.hpp"

namespace raytrace2::cpu {

struct BVHNode;

struct Scene {
  HittableList hittable_list;
  std::vector<MaterialVariant> materials;
  texture::TexArray textures;
  Camera cam;
  std::string cam_name;
};

}  // namespace raytrace2::cpu
