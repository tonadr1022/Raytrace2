#pragma once

namespace raytrace2::cpu {

// TODO: material namespace
struct MaterialMetal;
struct MaterialLambertian;
struct MaterialDielectric;
struct MaterialTexture;
struct DiffuseLight;
struct MaterialIsotropic;

using MaterialVariant = std::variant<MaterialMetal, MaterialLambertian, MaterialDielectric,
                                     MaterialTexture, DiffuseLight, MaterialIsotropic>;

namespace texture {
struct SolidColor;
struct Checker;
struct Noise;
using TextureVariant = std::variant<SolidColor, Checker, Noise>;
using TexArray = std::vector<TextureVariant>;
}  // namespace texture

}  // namespace raytrace2::cpu
