#pragma once

#include <glm/ext/vector_int2.hpp>

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
  vec3 background_color{1};
  glm::ivec2 dims{0, 0};
};

}  // namespace raytrace2::cpu
