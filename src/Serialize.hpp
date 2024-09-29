#pragma once

namespace raytrace2 {
struct CameraSettings;
struct AppSettings;
namespace cpu {
struct Scene;
}
}  // namespace raytrace2

namespace raytrace2::serialize {

std::optional<cpu::Scene> LoadScene(const std::string& filepath);
void WriteScene(const cpu::Scene& scene, const std::string& filepath);

AppSettings LoadAppSettings(const std::string& filepath);
CameraSettings LoadCameraSettings(const std::string& filepath);
void WriteCameraSettings(const CameraSettings& settings, const std::string& filepath);

}  // namespace raytrace2::serialize
