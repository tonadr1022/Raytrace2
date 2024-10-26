#pragma once

#include <nlohmann/json_fwd.hpp>

#include "Defs.hpp"

namespace raytrace2 {
namespace cpu {
class Camera;
}
struct AppSettings;
namespace cpu {
struct Scene;
struct Hittable;
struct Transform;
}  // namespace cpu
}  // namespace raytrace2

namespace raytrace2::serialize {

struct SceneLoader {
  [[nodiscard]] std::optional<cpu::Scene> LoadScene(const std::string& filepath);

 private:
  std::string filepath_;
  void PrintSceneError(const std::string& msg) const;
  std::shared_ptr<cpu::Hittable> ParseNode(std::vector<std::shared_ptr<cpu::Hittable>>& list,
                                           const nlohmann::json& node) const;
  [[nodiscard]] std::optional<mat4> ParseTransform(const nlohmann::json& node) const;
  [[nodiscard]] mat4 AccumulateTransform(const mat4& transform, const nlohmann::json& node) const;
};

AppSettings LoadAppSettings(const std::string& filepath);
cpu::Camera LoadCamera(const std::string& filepath);
void WriteCamera(const cpu::Camera& cam, const std::string& filepath);

}  // namespace raytrace2::serialize
