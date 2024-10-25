#include "Serialize.hpp"

#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <limits>
#include <nlohmann/json.hpp>

#include "Defs.hpp"
#include "Paths.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "cpu_raytrace/BVH.hpp"
#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/Fwd.hpp"
#include "cpu_raytrace/Hittable.hpp"
#include "cpu_raytrace/HittableList.hpp"
#include "cpu_raytrace/Material.hpp"
#include "cpu_raytrace/Quad.hpp"
#include "cpu_raytrace/Ray.hpp"
#include "cpu_raytrace/Scene.hpp"
#include "cpu_raytrace/Sphere.hpp"
#include "cpu_raytrace/Texture.hpp"
#include "cpu_raytrace/Transform.hpp"

namespace raytrace2::serialize {

namespace {
vec3 ToVec3(const std::array<float, 3>& arr) { return {arr[0], arr[1], arr[2]}; }
vec4 ToVec4(const std::array<float, 4>& arr) { return {arr[0], arr[1], arr[2], arr[3]}; }
std::array<float, 3> ToVec3Arr(const vec3& vec) { return {vec[0], vec[1], vec[2]}; }
}  // namespace

cpu::Camera LoadCamera(const nlohmann::json& obj) {
  cpu::Camera cam;
  cam.SetFOV(obj.value("fov", 90));
  cam.SetCenter(ToVec3(obj.value("center", std::array<float, 3>({0, 0, 1}))));
  cam.SetLookAt(ToVec3(obj.value("look_at", std::array<float, 3>({0, 0, 0}))));
  cam.SetDefocusAngle(obj.value("defocus_angle", 0.0f));
  cam.SetFocusDistance(obj.value("focus_distance", 1.f));
  return cam;
}

cpu::Camera LoadCamera(const std::string& filepath) {
  nlohmann::json obj = util::LoadJsonFile(filepath);
  return LoadCamera(obj);
}

void WriteCamera(const cpu::Camera& cam, const std::string& filepath) {
  nlohmann::json obj = {{"fov", cam.vfov_},
                        {"center", ToVec3Arr(cam.center_)},
                        {"look_at", ToVec3Arr(cam.lookat_)},
                        {"defocus_angle", cam.defocus_angle_},
                        {"focus_distance", cam.focus_dist_}};
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

std::shared_ptr<cpu::Hittable> ParseTransform(const std::shared_ptr<cpu::Hittable>& obj,
                                              const nlohmann::json& json_obj) {
  bool has_transform = false;
  if (json_obj.contains("transform")) {
    const auto& transform_json = json_obj["transform"];
    if (transform_json.is_object()) {
      vec3 translation{0};
      if (transform_json.contains("translation")) {
        has_transform = true;
        translation = ToVec3(transform_json.value("translation", std::array<float, 3>{0, 0, 0}));
      }
      glm::quat rotation;
      if (transform_json.contains("rotation")) {
        has_transform = true;
        vec4 angle_axis =
            ToVec4(transform_json.value("rotation", std::array<float, 4>({0, 0, 0, 0})));
        rotation = glm::angleAxis(glm::radians(angle_axis[0]),
                                  vec3{angle_axis[1], angle_axis[2], angle_axis[3]});
      }
      vec3 scale{1};
      if (transform_json.contains("scale")) {
        has_transform = true;
        scale = ToVec3(transform_json.value("scale", std::array<float, 3>({1, 1, 1})));
      }
      if (has_transform) {
        return std::make_shared<cpu::Transform>(obj, translation, rotation, scale);
      }
    } else {
      // TODO: make accessible here for better usability
      // print_scene_error("transform key must be an object");
    }
  }
  return obj;
}

std::optional<cpu::Scene> LoadScene(const std::string& filepath) {
  cpu::Scene scene;
  nlohmann::json obj = util::LoadJsonFile(filepath);
  auto cam_data = obj["camera"];
  scene.background_color = ToVec3(obj.value("background_color", std::array<float, 3>({1, 1, 1})));
  if (cam_data.is_object()) {
    scene.cam = LoadCamera(cam_data);
  } else {
    std::string camera_filename = cam_data.get<std::string>() + ".json";
    scene.cam = LoadCamera(GET_PATH("data/") + camera_filename);
    scene.cam_name = camera_filename;
  }
  auto primitives = obj["primitives"];
  auto json_spheres = primitives["spheres"];

  auto print_scene_error = [&](const std::string& str) {
    std::cerr << "Failed to parse Scene: " << str << ". " << filepath << '\n';
  };

  nlohmann::json::array_t json_materials = obj["materials"];
  auto json_textures = obj["textures"];
  size_t num_materials = json_materials.size();

  if (json_textures.is_array()) {
    for (const nlohmann::json& json_mat : json_textures) {
      std::string type = json_mat.value("type", "");
      cpu::texture::TextureVariant tex;
      if (type == "solid_color") {
        tex = cpu::texture::SolidColor{
            .albedo = ToVec3(json_mat.value("albedo", std::array<float, 3>{1, 1, 1}))};
      } else if (type == "checker") {
        tex =
            cpu::texture::Checker{json_mat.value("scale", 1.0f), json_mat.value("even_tex_idx", 0u),
                                  json_mat.value("odd_tex_idx", 0u)};
      } else if (type == "noise") {
        tex = cpu::texture::Noise{
            .noise =
                cpu::PerlinNoiseGen{
                    json_mat.value("point_count", 256),
                },
            .albedo = ToVec3(json_mat.value("albedo", std::array<float, 3>{1, 1, 1})),
            .scale = json_mat.value("scale", 1.0f),
            .noise_type = static_cast<cpu::texture::NoiseType>(
                json_mat.value("noise_type", static_cast<int>(cpu::texture::NoiseType::kMarble)))};
      } else {
        print_scene_error("Invalid texture type: " + type);
      }
      scene.textures.emplace_back(tex);
    }
  }

  std::unordered_map<size_t, uint32_t> id_to_arr_idx;
  for (const nlohmann::json& json_mat : json_materials) {
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
    } else if (type == "texture") {
      if (json_mat.contains("tex_idx")) {
        mat = cpu::MaterialTexture{.tex_idx = json_mat.value("tex_idx", 0u)};
      } else if (json_mat.contains("albedo")) {
        mat = cpu::MaterialTexture{.tex_idx = static_cast<uint32_t>(scene.textures.size())};
        scene.textures.emplace_back(cpu::texture::SolidColor{
            .albedo = ToVec3(json_mat.value("albedo", std::array<float, 3>({1, 1, 1})))});
      } else {
        print_scene_error("invalid texture, must contain tex_idx or albedo");
      }
    } else if (type == "diffuse_light") {
      if (json_mat.contains("tex_idx")) {
        mat = cpu::DiffuseLight{.tex_idx = json_mat.value("tex_idx", 0u)};
      } else if (json_mat.contains("albedo")) {
        mat = cpu::DiffuseLight{.tex_idx = static_cast<uint32_t>(scene.textures.size())};
        scene.textures.emplace_back(cpu::texture::SolidColor{
            .albedo = ToVec3(json_mat.value("albedo", std::array<float, 3>({1, 1, 1})))});
      } else {
        print_scene_error("invalid diffuse light, must contain tex_idx or albedo");
      }
    } else {
      print_scene_error("Invalid material type");
    }
    id_to_arr_idx[id] = scene.materials.size();
    scene.materials.emplace_back(mat);
  }

  for (const nlohmann::json& json_sphere : json_spheres) {
    std::array<float, 3> center = json_sphere.value("center", std::array<float, 3>{0, 0, 0});
    std::array<float, 3> displacement =
        json_sphere.value("displacement", std::array<float, 3>{0, 0, 0});
    float radius = json_sphere.value("radius", 0.5);
    auto sphere =
        std::make_shared<cpu::Sphere>(vec3{center[0], center[1], center[2]},
                                      vec3{displacement[0], displacement[1], displacement[2]},
                                      radius, id_to_arr_idx[json_sphere.value("material_id", 0)]);
    scene.hittable_list.Add(ParseTransform(sphere, json_sphere));
  }

  auto json_quads = primitives["quads"];
  for (const nlohmann::json& json_quad : json_quads) {
    auto q = ToVec3(json_quad.value("q", std::array<float, 3>{0, 0, 0}));
    auto u = ToVec3(json_quad.value("u", std::array<float, 3>{1, 0, 0}));
    auto v = ToVec3(json_quad.value("v", std::array<float, 3>{0, 0, 1}));

    auto quad =
        std::make_shared<cpu::Quad>(q, u, v, id_to_arr_idx[json_quad.value("material_id", 0)]);
    scene.hittable_list.Add(ParseTransform(quad, json_quad));
  }
  auto json_boxes = primitives["boxes"];
  for (const nlohmann::json& json_box : json_boxes) {
    auto a = ToVec3(json_box.value("a", std::array<float, 3>{0, 0, 0}));
    auto b = ToVec3(json_box.value("b", std::array<float, 3>{1, 1, 1}));
    auto box =
        std::make_shared<cpu::HittableList>(cpu::MakeBox(a, b, json_box.value("material_id", 0)));
    scene.hittable_list.Add(ParseTransform(box, json_box));
  }

  if (obj["camera"].is_object()) {
    auto cam = obj["camera"];
    int width = cam.value("width", 0);
    float aspect_ratio = cam.value("aspect_ratio", 0.0f);
    if (width != 0 && aspect_ratio != 0.0f) {
      float height = width / aspect_ratio;
      std::cout << width << ' ' << height << "hello\n";
      scene.dims = {width, height};
    }
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

// TODO: rest of serialization write
// nlohmann::json::object_t Serialize(const cpu::texture::Checker& mat) {
//   return {{"even_tex_idx", mat.even_tex_idx},
//           {"odd_tex_idx", mat.odd_tex_idx},
//           {"scale", 1.f / mat.inv_scale}};
// }
//
// nlohmann::json::object_t Serialize(const cpu::texture::SolidColor& mat) {
//   return {{"albedo", ToVec3Arr(mat.albedo)}};
// }

nlohmann::json::object_t Serialize(const cpu::MaterialTexture& mat) {
  return {{"tex_idx", mat.tex_idx}};
}

nlohmann::json::object_t Serialize(const cpu::DiffuseLight& mat) {
  return {{"tex_idx", mat.tex_idx}};
}

nlohmann::json::object_t Serialize(const cpu::MaterialMetal& mat) {
  return {{"albedo", ToVec3Arr(mat.albedo)}, {"fuzz", mat.fuzz}, {"type", "metal"}};
}

}  // namespace

void WriteScene(const cpu::Scene& scene, const std::string& filepath) {
  nlohmann::json obj = nlohmann::json::object();
  nlohmann::json spheres = nlohmann::json::array();
  nlohmann::json materials = nlohmann::json::array();
  if (!scene.cam_name.empty()) {
    WriteCamera(scene.cam, GET_PATH("data/") + scene.cam_name + ".json");
  }

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

  for (const auto& sphere : scene.hittable_list.objects) {
    std::shared_ptr<cpu::Sphere> sphere_ptr = std::dynamic_pointer_cast<cpu::Sphere>(sphere);
    if (sphere_ptr) {
      nlohmann::json json_sphere = {
          {"center", ToVec3Arr(sphere_ptr->center_displacement.origin)},
          {"displacement", ToVec3Arr(sphere_ptr->center_displacement.direction)},
          {"radius", sphere_ptr->radius},
          {"material_id", sphere_ptr->material_handle}};
      spheres.push_back(json_sphere);
    }
  }
  obj["camera"] = scene.cam_name;
  obj["primitives"] = {{"spheres", spheres}};
  obj["materials"] = materials;

  util::WriteJson(obj, filepath);
}

}  // namespace raytrace2::serialize
