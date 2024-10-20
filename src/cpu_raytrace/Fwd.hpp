#pragma once

namespace raytrace2::cpu {

struct MaterialMetal;
struct MaterialLambertian;
struct MaterialDielectric;
struct MaterialTexture;
struct DiffuseLight;

using MaterialVariant = std::variant<MaterialMetal, MaterialLambertian, MaterialDielectric,
                                     MaterialTexture, DiffuseLight>;

namespace texture {
struct SolidColor;
struct Checker;
struct Noise;
using TextureVariant = std::variant<SolidColor, Checker, Noise>;
using TexArray = std::vector<TextureVariant>;
}  // namespace texture

}  // namespace raytrace2::cpu
