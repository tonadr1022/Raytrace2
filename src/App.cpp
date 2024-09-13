#include "App.hpp"

#include <SDL_events.h>
#include <SDL_timer.h>
#include <imgui.h>

#include <cstddef>

#include "Defs.hpp"
#include "Paths.hpp"
#include "Window.hpp"
#include "cpu_raytrace/Scene.hpp"
#include "gl/Buffer.hpp"
#include "gl/ShaderManager.hpp"
#include "gl/Texture.hpp"
#include "gl/VertexArray.hpp"
#include "pch.hpp"

namespace raytrace2 {

namespace {

struct PosTexVertex {
  glm::vec2 position;
  glm::vec2 tex_coords;
};

class Quad {
 public:
  void Init() {
    vao_.Init();
    vao_.EnableAttribute<float>(0, 3, offsetof(PosTexVertex, position));
    vao_.EnableAttribute<float>(1, 2, offsetof(PosTexVertex, tex_coords));
    const std::array<PosTexVertex, 4> quad_vertices_full = {
        {// positions        // texture Coords
         {glm::vec3{-1, 1, 0.0f}, glm::vec2{0.0f, 1.0f}},
         {glm::vec3{-1, -1, 0.0f}, glm::vec2{0.0f, 0.0f}},
         {glm::vec3{1, 1, 0.0f}, glm::vec2{1.0f, 1.0f}},
         {glm::vec3{1, -1, 0.0f}, glm::vec2{1.0f, 0.0f}}}};
    std::vector<uint32_t> indices = {0, 1, 2, 2, 1, 3};
    std::vector<PosTexVertex> vertices = {quad_vertices_full.begin(), quad_vertices_full.end()};
    vbo_.Init(sizeof(PosTexVertex) * 4, 0, vertices.data());
    ebo_.Init(sizeof(uint32_t) * 6, 0, indices.data());
    vao_.AttachVertexBuffer(vbo_.Id(), 0, 0, sizeof(PosTexVertex));
    vao_.AttachElementBuffer(ebo_.Id());
  }
  void Bind() { vao_.Bind(); }
  void Draw() { glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); }

 private:
  gl::VertexArray vao_;
  gl::Buffer<PosTexVertex> vbo_;
  gl::Buffer<uint32_t> ebo_;
};

std::unique_ptr<gl::Texture> output_tex;
glm::ivec2 viewport_dims;

}  // namespace

void App::OnResize(glm::ivec2 dims) {
  viewport_dims = dims;
  output_tex = std::make_unique<gl::Texture>(gl::Tex2DCreateInfoEmpty{.dims = dims,
                                                                      .wrap_s = GL_REPEAT,
                                                                      .wrap_t = GL_REPEAT,
                                                                      .internal_format = GL_RGB8,
                                                                      .min_filter = GL_NEAREST,
                                                                      .mag_filter = GL_NEAREST});
  rt_.OnResize(dims);
}

void App::Run() {
  Window window{1600, 900, "raytrace_2", [this](SDL_Event& event) { OnEvent(event); }};

  gl::ShaderManager::Init();
  gl::ShaderManager::Get().AddShader(
      "textured_quad", {{GET_SHADER_PATH("textured_quad.vs.glsl"), gl::ShaderType::kVertex, {}},
                        {GET_SHADER_PATH("textured_quad.fs.glsl"), gl::ShaderType::kFragment, {}}});

  Quad quad;
  quad.Init();
  bool imgui_enabled{true};
  OnResize(window.GetWindowSize());
  rt_.camera.SetCenter(glm::vec3(0));

  GLuint fbo;
  glCreateFramebuffers(1, &fbo);
  auto shader = gl::ShaderManager::Get().GetShader("textured_quad").value();
  shader.Bind();
  quad.Bind();

  uint64_t curr_time = SDL_GetPerformanceCounter();
  uint64_t prev_time = 0;
  double dt = 0;

  cpu::Scene scene;
  scene.spheres.emplace_back(vec3{0, 0, -1}, 0.5);
  scene.spheres.emplace_back(vec3{0, -100.5, -1}, 100);

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

    rt_.Update(scene);

    window.StartRenderFrame(imgui_enabled);
    ImGui::Begin("Settings");
    bool vsync = window.GetVsync();
    if (ImGui::Checkbox("Vsync", &vsync)) {
      window.SetVsync(vsync);
    }
    ImGui::End();

    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    output_tex->Bind(0);
    glTextureSubImage2D(output_tex->Id(), 0, 0, 0, viewport_dims.x, viewport_dims.y, GL_RGB,
                        GL_UNSIGNED_BYTE, rt_.Pixels().data());
    quad.Draw();

    window.EndRenderFrame(imgui_enabled);
  }
}

void App::OnEvent(SDL_Event& event) {
  if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
    OnResize(glm::ivec2{event.window.data1, event.window.data2});
  }
}

}  // namespace raytrace2
