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
  if (!is_scattered) return emission_color;

  real scattering_pdf = std::visit(
      [&](auto&& material) { return material.ScatteringPDF(r, rec, scattered); }, *rec.material);

  vec3 color_from_scatter =
      (attenuation * scattering_pdf * RayColor(scattered, depth - 1, scene)) / scattering_pdf;

  return color_from_scatter + emission_color;
}

}  // namespace

void RayTracer::Reset() {
  accumulation_data_.clear();
  accumulation_data_.resize(static_cast<size_t>(dims_.x) * dims_.y);
  frame_idx_ = 0;
}

#define STRATIFY
#ifdef STRATIFY
void RayTracer::Update(const Scene& scene) {
  camera->Update();
  int samples_per_pix = camera->SamplesPerPixel();
  int sqrt_samples_per_pix = camera->SqrtSamplesPerPixel();
  // get s_j and s_i for this frame
  int s_i = frame_idx_ % sqrt_samples_per_pix;
  int s_j = frame_idx_ / sqrt_samples_per_pix % sqrt_samples_per_pix;
  frame_idx_++;
  auto per_pixel = [this, &scene, s_j, s_i](const glm::ivec3& idx) {
    vec3 ray_color = RayColor(camera->GetRay(idx.x, idx.y, s_i, s_j), max_depth, scene);
    accumulation_data_[idx.z] += ray_color;
    pixels_[idx.z] = ToColor(glm::clamp(accumulation_data_[idx.z] / static_cast<real>(frame_idx_),
                                        static_cast<real>(0.0), static_cast<real>(1.0)));
  };

  std::for_each(std::execution::par, iter_.begin(), iter_.end(), per_pixel);
}
#else
void RayTracer::Update(const Scene& scene) {
  camera->Update();
  frame_idx_++;
  auto per_pixel = [this, &scene](const glm::ivec3& idx) {
    vec3 ray_color = RayColor(camera->GetRay(idx.x, idx.y), max_depth, scene);
    accumulation_data_[idx.z] += ray_color;
    pixels_[idx.z] = ToColor(glm::clamp(accumulation_data_[idx.z] / static_cast<real>(frame_idx_),
                                        static_cast<real>(0.0), static_cast<real>(1.0)));
  };

  std::for_each(std::execution::par, iter_.begin(), iter_.end(), per_pixel);
}
#endif

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
void RayTracer::OnResize(glm::ivec2 dims) {
  dims_ = dims;
  camera->SetDims(dims);
  frame_idx_ = 0;

  size_t new_size = static_cast<size_t>(dims.x) * dims.y;
  pixels_.resize(new_size);
  iter_.resize(new_size);
  accumulation_data_.resize(new_size);

  int i = 0;
  for (int y = 0; y < dims.y; y++) {
    for (int x = 0; x < dims.x; x++, i++) {
      iter_[i] = {x, y, i};
    }
  }
  Reset();
}
std::vector<vec3> RayTracer::NonConvertedPixels() const {
  std::vector<vec3> ret(accumulation_data_.size());
  for (size_t i = 0; i < ret.size(); i++) {
    ret[i] = vec3{accumulation_data_[i].x / frame_idx_, accumulation_data_[i].y / frame_idx_,
                  accumulation_data_[i].z / frame_idx_};
  }
  return ret;
}
}  // namespace raytrace2::cpu
