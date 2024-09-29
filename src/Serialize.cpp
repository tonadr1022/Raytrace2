#include "Serialize.hpp"

#include <limits>
#include <nlohmann/json.hpp>

#include "Defs.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "cpu_raytrace/Fwd.hpp"
#include "cpu_raytrace/Material.hpp"
#include "cpu_raytrace/Scene.hpp"
#include "cpu_raytrace/Sphere.hpp"

namespace raytrace2::serialize {

namespace {
vec3 ToVec3(const std::array<float, 3>& arr) { return {arr[0], arr[1], arr[2]}; }
std::array<float, 3> ToVec3Arr(const vec3& vec) { return {vec[0], vec[1], vec[2]}; }
}  // namespace

CameraSettings LoadCameraSettings(const std::string& filepath) {
  nlohmann::json obj = util::LoadJsonFile(filepath);
  CameraSettings settings{.fov = obj.value("fov", 90.f)};
  return settings;
}

void WriteCameraSettings(const CameraSettings& settings, const std::string& filepath) {
  nlohmann::json obj;
  obj["fov"] = settings.fov;
  util::WriteJson(obj, filepath);
}

AppSettings LoadAppSettings(const std::string& filepath) {
  AppSettings settings;
  nlohmann::json obj = util::LoadJsonFile(filepath);
  settings.num_samples = obj.value("num_samples", 1);
  settings.render_once = obj.value("render_once", false);
  settings.save_after_render_once = obj.value("save_after_render_once", false);
  return settings;
}

std::optional<cpu::Scene> LoadScene(const std::string& filepath) {
  cpu::Scene scene;
  nlohmann::json obj = util::LoadJsonFile(filepath);
  auto primitives = obj["primitives"];
  auto json_spheres = primitives["spheres"];

  nlohmann::json::array_t json_materials = obj["materials"];
  size_t num_materials = json_materials.size();

  std::unordered_map<size_t, uint32_t> id_to_arr_idx;
  for (const nlohmann::json& json_mat : json_materials) {
    auto print_scene_error = [&](const std::string& str) {
      std::cerr << "Failed to parse Scene: " << str << ". " << filepath << '\n';
    };

    size_t id = json_mat.value("id", std::numeric_limits<size_t>::max());
    if (id == std::numeric_limits<size_t>::max()) {
      print_scene_error("material id not found");
      return {};
    }
    if (id >= num_materials) {
      print_scene_error("invalid material id, must be: 0 <= id < length of materials array");
      return {};
    }

    std::string type = json_mat.value("type", "");
    if (type.empty()) {
      print_scene_error("material type field empty");
      return {};
    }

    cpu::MaterialVariant mat;
    if (type == "lambertian") {
      mat = cpu::MaterialLambertian{
          .albedo = ToVec3(json_mat.value("albedo", std::array<float, 3>{1, 1, 1}))};

    } else if (type == "dielectric") {
      mat = cpu::MaterialDielectric{.refraction_index = json_mat.value("refraction_index", 1.0f)};
    } else if (type == "metal") {
      mat = cpu::MaterialMetal{
          .albedo = ToVec3(json_mat.value("albedo", std::array<float, 3>{1, 1, 1})),
          .fuzz = json_mat.value("fuzz", 0.0f)};
    }
    id_to_arr_idx[id] = scene.materials.size();
    scene.materials.emplace_back(mat);
  }

  for (const nlohmann::json& json_sphere : json_spheres) {
    std::array<float, 3> center = json_sphere.value("center", std::array<float, 3>{0, 0, 0});
    float radius = json_sphere.value("radius", 0.5);
    scene.spheres.emplace_back(
        cpu::Sphere{.center = vec3{center[0], center[1], center[2]},
                    .radius = radius,
                    .material_handle = id_to_arr_idx[json_sphere.value("material_id", 0)]});
  }

  return scene;
}

namespace {

nlohmann::json::object_t Serialize(const cpu::MaterialDielectric& mat) {
  return {{"refraction_index", mat.refraction_index}, {"type", "dielectric"}};
}

nlohmann::json::object_t Serialize(const cpu::MaterialLambertian& mat) {
  return {{"albedo", ToVec3Arr(mat.albedo)}, {"type", "lambertian"}};
}

nlohmann::json::object_t Serialize(const cpu::MaterialMetal& mat) {
  return {{"albedo", ToVec3Arr(mat.albedo)}, {"fuzz", mat.fuzz}, {"type", "metal"}};
}

}  // namespace

void WriteScene(const cpu::Scene& scene, const std::string& filepath) {
  nlohmann::json obj = nlohmann::json::object();
  nlohmann::json spheres = nlohmann::json::array();
  nlohmann::json materials = nlohmann::json::array();

  size_t i = 0;
  for (const auto& mat : scene.materials) {
    materials.push_back(std::visit(
        [&](const auto& material) {
          nlohmann::json::object_t obj = Serialize(material);
          obj["id"] = i;
          return obj;
        },
        mat));
    i++;
  }

  for (const auto& sphere : scene.spheres) {
    nlohmann::json json_sphere = {{"center", ToVec3Arr(sphere.center)},
                                  {"radius", sphere.radius},
                                  {"material_id", sphere.material_handle}};
    spheres.push_back(json_sphere);
  }
  obj["primitives"] = {{"spheres", spheres}};
  obj["materials"] = materials;

  util::WriteJson(obj, filepath);
}

}  // namespace raytrace2::serialize
