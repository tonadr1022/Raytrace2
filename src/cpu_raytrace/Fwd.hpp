#pragma once

namespace raytrace2::cpu {

struct MetalMaterial;
struct LambertianMaterial;
struct DielectricMaterial;

using MaterialVariant = std::variant<MetalMaterial, LambertianMaterial, DielectricMaterial>;

}  // namespace raytrace2::cpu
