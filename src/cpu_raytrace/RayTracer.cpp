#include "RayTracer.hpp"

#include <imgui.h>

#include <execution>

#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/Material.hpp"
#include "cpu_raytrace/Math.hpp"
#include "cpu_raytrace/Scene.hpp"

namespace raytrace2::cpu {

namespace {

constexpr int kMaxDepth = 50;

color ToColor(const vec3& col) {
  return color{floor(col.x * 255.999), floor(col.y * 255.999), floor(col.z * 255.999), 255};
}

float col = 0.5f;
vec3 RayColor(const cpu::Ray& r, int depth, const Scene& scene) {
  vec3 attenutation{1};
  Ray scattered = r;
  while (depth > 0) {
    cpu::HitRecord rec;
    bool hit_any_sphere =
        scene.hittable_list.Hit(scene, scattered, cpu::Interval{0.001, kInfinity}, rec);
    // bool hit_any_sphere = HitAny(scene, std::span<cpu::Sphere const>(scene.spheres), scattered,
    //                              cpu::Interval{0.001, kInfinity}, rec);
    if (hit_any_sphere) {
      vec3 local_attenuation;
      EASSERT(rec.material != nullptr);
      if (Scatter(scene.textures, *rec.material, scattered, rec, local_attenuation, scattered)) {
        attenutation *= local_attenuation;
      } else {
        // no scatter, so absorb the ray
        return vec3{0};
      }
    } else {
      // calc background and break if no hit
      vec3 unit_dir = glm::normalize(scattered.direction);
      float a = 0.5f * (unit_dir.y + 1.0);
      attenutation *= (1.0f - a) * vec3{1, 1, 1} + a * vec3{0.5, 0.7, 1.0};
      break;
    }

    depth--;
  }
  return attenutation;
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
    vec3 ray_color = math::LinearToGamma(RayColor(camera->GetRay(idx.x, idx.y), kMaxDepth, scene));
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
  if (ImGui::SliderFloat("Col", &col, 0.0f, 1.0f)) {
    Reset();
  }
  ImGui::End();
}
}  // namespace raytrace2::cpu
