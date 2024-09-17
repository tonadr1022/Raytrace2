#pragma once

namespace raytrace2::cpu {

struct MetalMaterial;
struct LambertianMaterial;

using MaterialVariant = std::variant<MetalMaterial, LambertianMaterial>;

}  // namespace raytrace2::cpu
