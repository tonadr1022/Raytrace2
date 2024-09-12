#include "App.hpp"

#include <SDL_events.h>
#include <SDL_timer.h>
#include <imgui.h>

#include <cstddef>
#include <execution>

#include "Paths.hpp"
#include "Window.hpp"
#include "gl/Buffer.hpp"
#include "gl/ShaderManager.hpp"
#include "gl/Texture.hpp"
#include "gl/VertexArray.hpp"
#include "pch.hpp"
#include "raytrace/Ray.hpp"

namespace raytrace2 {

using color = glm::u8vec3;
using vec3 = glm::dvec3;
namespace {

struct PosTexVertex {
  glm::vec2 position;
  glm::vec2 tex_coords;
};
color ToColor(const vec3& col) {
  return color{floor(col.x * 255.999), floor(col.y * 255.999), floor(col.z * 255.999)};
}

bool HitSphere(const vec3& center, double radius, const Ray& r) {
  vec3 oc = center - r.origin;
  double a = glm::dot(r.direction, r.direction);
  double b = -2.0 * glm::dot(r.direction, oc);
  double c = glm::dot(oc, oc) - radius * radius;
  double discriminant = b * b - 4 * a * c;
  return discriminant >= 0;

  return false;
}

color RayColor(const Ray& r) {
  if (HitSphere(vec3{0, 0, -1}, 0.5, r)) {
    return ToColor({1, 1, 0});
  }

  vec3 unit_dir = glm::normalize(r.direction);
  auto a = 0.5 * (unit_dir.y + 1.0);
  vec3 col = (1.0 - a) * vec3{1, 1, 1} + a * vec3{0.5, 0.7, 1.0};
  return ToColor(col);
}

}  // namespace

void App::Run() {
  Window window{1600, 900, "raytrace_2", [this](SDL_Event& event) { OnEvent(event); }};

  gl::ShaderManager::Init();
  gl::ShaderManager::Get().AddShader(
      "textured_quad", {{GET_SHADER_PATH("textured_quad.vs.glsl"), gl::ShaderType::kVertex, {}},
                        {GET_SHADER_PATH("textured_quad.fs.glsl"), gl::ShaderType::kFragment, {}}});

  gl::VertexArray quad_vao;
  gl::Buffer<PosTexVertex> quad_vbo;
  gl::Buffer<uint32_t> quad_ebo;

  quad_vao.Init();
  quad_vao.EnableAttribute<float>(0, 3, offsetof(PosTexVertex, position));
  quad_vao.EnableAttribute<float>(1, 2, offsetof(PosTexVertex, tex_coords));

  const std::array<PosTexVertex, 4> quad_vertices_full = {
      {// positions        // texture Coords
       {glm::vec3{-1, 1, 0.0f}, glm::vec2{0.0f, 1.0f}},
       {glm::vec3{-1, -1, 0.0f}, glm::vec2{0.0f, 0.0f}},
       {glm::vec3{1, 1, 0.0f}, glm::vec2{1.0f, 1.0f}},
       {glm::vec3{1, -1, 0.0f}, glm::vec2{1.0f, 0.0f}}}};
  std::vector<uint32_t> indices = {0, 1, 2, 2, 1, 3};
  std::vector<PosTexVertex> vertices = {quad_vertices_full.begin(), quad_vertices_full.end()};
  quad_vbo.Init(sizeof(PosTexVertex) * 4, 0, vertices.data());
  quad_ebo.Init(sizeof(uint32_t) * 6, 0, indices.data());
  quad_vao.AttachVertexBuffer(quad_vbo.Id(), 0, 0, sizeof(PosTexVertex));
  quad_vao.AttachElementBuffer(quad_ebo.Id());

  bool imgui_enabled{true};
  auto dims = window.GetWindowSize();
  gl::Texture tex{gl::Tex2DCreateInfoEmpty{.dims = dims,
                                           .wrap_s = GL_REPEAT,
                                           .wrap_t = GL_REPEAT,
                                           .internal_format = GL_RGB8,
                                           .min_filter = GL_NEAREST,
                                           .mag_filter = GL_NEAREST}};

  std::vector<glm::u8vec3> pixels(static_cast<size_t>(dims.x * dims.y));
  int i = 0;
  for (int y = 0; y < dims.y; y++) {
    for (int x = 0; x < dims.x; x++, i++) {
      pixels[i] = {floor((static_cast<double>(y) / static_cast<double>(dims.y)) * 255.999),
                   floor((static_cast<double>(x) / static_cast<double>(dims.x)) * 255.999), 0};
    }
  }

  GLuint fbo;
  glCreateFramebuffers(1, &fbo);
  auto shader = gl::ShaderManager::Get().GetShader("textured_quad").value();
  shader.Bind();
  tex.Bind(0);
  quad_vao.Bind();

  double focal_length = 1.0;
  double viewport_height = 2.0;
  double viewport_width = viewport_height * (static_cast<double>(dims.x) / dims.y);
  vec3 camera_center(0);

  vec3 viewport_length_u = vec3(viewport_width, 0, 0);
  vec3 viewport_length_v = vec3(0, viewport_height, 0);

  vec3 pixel_delta_u = viewport_length_u / static_cast<double>(dims.x);
  vec3 pixel_delta_v = viewport_length_v / static_cast<double>(dims.y);

  vec3 viewport_upper_left =
      camera_center - vec3(0, 0, focal_length) - viewport_length_u / 2. - viewport_length_v / 2.;
  vec3 pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

  uint64_t curr_time = SDL_GetPerformanceCounter();
  uint64_t prev_time = 0;
  double dt = 0;

  std::vector<glm::ivec2> iter(static_cast<size_t>(dims.x * dims.y));
  i = 0;
  for (int y = 0; y < dims.y; y++) {
    for (int x = 0; x < dims.x; x++, i++) {
      iter[i] = {x, y};
    }
  }

  while (!window.ShouldClose()) {
    ZoneScoped;
    prev_time = curr_time;
    curr_time = SDL_GetPerformanceCounter();
    dt = ((curr_time - prev_time) / static_cast<double>(SDL_GetPerformanceFrequency()));
    static double sum = 0;
    static int frame_counter_count = 0;
    sum += dt;
    frame_counter_count++;
    if (frame_counter_count % 20 == 0) {
      window.SetTitle("Frame Time:" + std::to_string(sum / frame_counter_count) +
                      ", FPS: " + std::to_string(frame_counter_count / sum));
      frame_counter_count = 0;
      sum = 0;
    }

    window.PollEvents();

    auto per_pixel = [pixel00_loc, pixel_delta_u, pixel_delta_v, camera_center, &pixels,
                      dims](glm::ivec2& idx) {
      auto pixel_center = pixel00_loc + (static_cast<double>(idx.x) * pixel_delta_u) +
                          (static_cast<double>(idx.y) * pixel_delta_v);
      auto ray_dir = pixel_center - camera_center;
      Ray r{.origin = camera_center, .direction = ray_dir};
      pixels[idx.y * dims.x + idx.x] = RayColor(r);
    };

    static int parallel = 0;
    if (parallel == 0) {
      std::for_each(std::execution::par, iter.begin(), iter.end(), per_pixel);
    } else {
      std::for_each(iter.begin(), iter.end(), per_pixel);
    }

    window.StartRenderFrame(imgui_enabled);
    ImGui::Begin("Settings");
    ImGui::SliderInt("Option", &parallel, 0, 1);
    bool vsync = window.GetVsync();
    if (ImGui::Checkbox("Vsync", &vsync)) {
      window.SetVsync(vsync);
    }
    ImGui::End();

    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glTextureSubImage2D(tex.Id(), 0, 0, 0, dims.x, dims.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    window.EndRenderFrame(imgui_enabled);
  }
}

void App::OnEvent(SDL_Event&) {}

}  // namespace raytrace2
