#include "RayTracer.hpp"

#include <imgui.h>

#include <execution>

#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/Material.hpp"
#include "cpu_raytrace/Math.hpp"
#include "cpu_raytrace/Scene.hpp"

namespace raytrace2::cpu {

namespace {

color ToColor(const vec3& col) {
  return color{floor(col.x * 255.999), floor(col.y * 255.999), floor(col.z * 255.999), 255};
}

vec3 RayColor(const cpu::Ray& r, int depth, const Scene& scene) {
  if (depth <= 0) return {0, 0, 0};

  HitRecord rec;

  if (!scene.hittable_list.Hit(scene, r, cpu::Interval{0.001, kInfinity}, rec)) {
    return scene.background_color;
  }

  Ray scattered;
  vec3 attenuation;

  vec3 emission_color =
      std::visit([&](auto&& material) { return material.Emit(scene.textures, rec.uv, rec.point); },
                 *rec.material);

  bool is_scattered = std::visit(
      [&](auto&& material) {
        return material.Scatter(scene.textures, r, rec, attenuation, scattered);
      },
      *rec.material);
  if (is_scattered) {
    return attenuation * RayColor(scattered, depth - 1, scene) + emission_color;
  }
  return emission_color;
}

}  // namespace

void RayTracer::Reset() {
  accumulation_data_.clear();
  accumulation_data_.resize(static_cast<size_t>(dims_.x) * dims_.y);
  frame_idx_ = 0;
}

void RayTracer::Update(const Scene& scene) {
  frame_idx_++;
  auto per_pixel = [this, &scene](const glm::ivec3& idx) {
    vec3 ray_color = math::LinearToGamma(RayColor(camera->GetRay(idx.x, idx.y), max_depth, scene));
    accumulation_data_[idx.z] += ray_color;
    pixels_[idx.z] =
        ToColor(glm::clamp(accumulation_data_[idx.z] / static_cast<float>(frame_idx_), 0.0f, 1.0f));
  };

  std::for_each(std::execution::par, iter_.begin(), iter_.end(), per_pixel);
}

bool RayTracer::OnEvent(const SDL_Event& event) {
  if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
    OnResize(glm::ivec2{event.window.data1, event.window.data2});
    return true;
  }
  return false;
}

void RayTracer::OnImGui() {
  ImGui::Begin("Settings");
  if (ImGui::Button("Reset")) {
    Reset();
  }
  ImGui::End();
}
}  // namespace raytrace2::cpu
