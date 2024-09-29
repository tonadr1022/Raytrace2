#pragma once

namespace raytrace2::cpu {

struct MaterialMetal;
struct MaterialLambertian;
struct MaterialDielectric;

using MaterialVariant = std::variant<MaterialMetal, MaterialLambertian, MaterialDielectric>;

}  // namespace raytrace2::cpu
