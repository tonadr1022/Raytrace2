#pragma once

namespace raytrace2 {
namespace cpu {
class Camera;
}
struct AppSettings;
namespace cpu {
struct Scene;
}
}  // namespace raytrace2

namespace raytrace2::serialize {

std::optional<cpu::Scene> LoadScene(const std::string& filepath);
void WriteScene(const cpu::Scene& scene, const std::string& filepath);

AppSettings LoadAppSettings(const std::string& filepath);
cpu::Camera LoadCamera(const std::string& filepath);
void WriteCamera(const cpu::Camera& cam, const std::string& filepath);

}  // namespace raytrace2::serialize
