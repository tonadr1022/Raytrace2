#include "App.hpp"

#include <SDL_events.h>
#include <SDL_timer.h>
#include <imgui.h>

#include <cstddef>
#include <filesystem>
#include <memory>

#include "Paths.hpp"
#include "Serialize.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "Window.hpp"
#include "cpu_raytrace/Camera.hpp"
#include "cpu_raytrace/Scene.hpp"
#include "cpu_raytrace/Sphere.hpp"
#include "cpu_raytrace/Texture.hpp"
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
  if (settings_.render_window) {
    output_tex = std::make_unique<gl::Texture>(gl::Tex2DCreateInfoEmpty{.dims = dims,
                                                                        .wrap_s = GL_REPEAT,
                                                                        .wrap_t = GL_REPEAT,
                                                                        .internal_format = GL_RGBA8,
                                                                        .min_filter = GL_NEAREST,
                                                                        .mag_filter = GL_NEAREST});
  }
  cpu_tracer_.OnResize(dims);
}

void App::Run(int argc, char* argv[]) {
  settings_ = serialize::LoadAppSettings(GET_PATH("data/settings.json"));

  std::string full_scene_path;
  std::string filename;
  if (argc == 1) {
    full_scene_path = GET_PATH("data/scene2.json");
    filename = "scene2";
  } else {
    full_scene_path = std::string(argv[1]);
    const std::string suffix = ".json";
    if (full_scene_path.length() >= suffix.length() &&
        full_scene_path.compare(full_scene_path.length() - suffix.length(), suffix.length(),
                                suffix) == 0) {
      filename = full_scene_path.substr(0, full_scene_path.length() - suffix.length());
    } else {
      filename = full_scene_path;
      full_scene_path += suffix;
    }
    full_scene_path = GET_PATH("data/") + full_scene_path;
  }
  bool user_defined_output_path = false;
  std::string image_output_path;
  // get output name
  if (argc == 3) {
    user_defined_output_path = true;
    image_output_path = argv[2];
  }
  std::cout << "Render window: " << settings_.render_window << '\n';
  std::cout << "Render once: " << settings_.render_once << '\n';
  std::cout << "Num Samples: " << settings_.num_samples << '\n';
  std::cout << "Max Depth: " << settings_.max_depth << '\n';
  std::cout << "Save Output: " << settings_.save_after_render_once << '\n';
  std::cout << "Scene Path: " << full_scene_path << '\n';

  glm::ivec2 initial_dims{1600, 900};
  serialize::SceneLoader loader;
  auto scene_opt = loader.LoadScene(full_scene_path);
  if (!scene_opt.has_value()) {
    exit(1);
  }
  cpu::Scene& scene = scene_opt.value();
  if (scene.dims.x != 0 && scene.dims.y != 0) {
    initial_dims = scene.dims;
    scene.cam.SetDims(scene.dims);
  }
  scene.hittable_list = cpu::HittableList{std::make_shared<cpu::BVHNode>(scene.hittable_list)};
  // TODO: streamline
  cpu_tracer_.max_depth = settings_.max_depth;
  scene.cam.SetSamplesPerPixel(settings_.num_samples);
  cpu_tracer_.camera = &scene.cam;

  if (settings_.render_window) {
    window_ = std::make_unique<Window>(initial_dims.x, initial_dims.y, "raytrace_2",
                                       [this](SDL_Event& event) { OnEvent(event); });
  }

  Quad quad;
  if (settings_.render_window) {
    gl::ShaderManager::Init();
    gl::ShaderManager::Get().AddShader(
        "textured_quad",
        {{GET_SHADER_PATH("textured_quad.vs.glsl"), gl::ShaderType::kVertex, {}},
         {GET_SHADER_PATH("textured_quad.fs.glsl"), gl::ShaderType::kFragment, {}}});
    gl::ShaderManager::Get().AddShader(
        "rtiow", {{GET_SHADER_PATH("textured_quad.vs.glsl"), gl::ShaderType::kVertex, {}},
                  {GET_SHADER_PATH("rtiow.fs.glsl"), gl::ShaderType::kFragment, {}}});
    quad.Init();

    initial_dims = window_->GetWindowSize();
    GLuint fbo;
    glCreateFramebuffers(1, &fbo);
    auto shader = gl::ShaderManager::Get().GetShader("textured_quad").value();
    shader.Bind();
    quad.Bind();
  }

  OnResize(initial_dims);

  uint64_t curr_time = SDL_GetPerformanceCounter();
  uint64_t prev_time = 0;
  double dt = 0;

  auto write_image = [&, this]() {
    if (!std::filesystem::exists(GET_PATH("output/"))) {
      std::filesystem::create_directory(GET_PATH("output/"));
    }
    if (!user_defined_output_path) {
      image_output_path = GET_PATH("output/") + filename + "_" + util::CurrentDateTime() + ".png";
    }
    std::cout << "Writing image: " << image_output_path << '\n';
    util::WriteImage(cpu_tracer_.NonConvertedPixels(), cpu_tracer_.Dims().x, cpu_tracer_.Dims().y,
                     image_output_path);
  };

  if (settings_.render_window) {
    while (!window_->ShouldClose()) {
      ZoneScoped;
      prev_time = curr_time;
      curr_time = SDL_GetPerformanceCounter();
      dt = ((curr_time - prev_time) / static_cast<double>(SDL_GetPerformanceFrequency()));
      if (settings_.render_window) {
        static double sum = 0;
        static int frame_counter_count = 0;
        sum += dt;
        frame_counter_count++;
        if (frame_counter_count % 20 == 0) {
          window_->SetTitle("Frame Time:" + std::to_string(sum / frame_counter_count) +
                            ", FPS: " + std::to_string(frame_counter_count / sum));
          frame_counter_count = 0;
          sum = 0;
        }

        window_->PollEvents();
      }

      bool done = (cpu_tracer_.FrameIdx() > settings_.num_samples);
      if (!done || !settings_.render_once) {
        cpu_tracer_.Update(scene);
      } else {
        if (settings_.save_after_render_once) {
          write_image();
        }
      }
      if (done && settings_.render_once) {
        break;
      }

      window_->StartRenderFrame(imgui_enabled_);

      ImGui::Begin("Settings");
      bool vsync = window_->GetVsync();
      if (ImGui::Checkbox("Vsync", &vsync)) {
        window_->SetVsync(vsync);
      }
      static char scene_name[100];
      ImGui::Text("Frame Count %i", static_cast<int>(cpu_tracer_.FrameIdx()));
      ImGui::InputText("##Scene Name", scene_name, 100);
      ImGui::SameLine();
      if (ImGui::Button("Load Scene")) {
        serialize::SceneLoader loader;
        auto scene_opt = loader.LoadScene(GET_PATH("data/") + std::string(scene_name));
        if (scene_opt.has_value()) {
          scene = scene_opt.value();
          cpu_tracer_.Reset();
        }
      }

      cpu_tracer_.OnImGui();

      glClearColor(0.1, 0.1, 0.1, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      output_tex->Bind(0);
      glTextureSubImage2D(output_tex->Id(), 0, 0, 0, viewport_dims.x, viewport_dims.y, GL_RGBA,
                          GL_UNSIGNED_BYTE, cpu_tracer_.Pixels().data());
      glEnable(GL_FRAMEBUFFER_SRGB);
      quad.Draw();
      glDisable(GL_FRAMEBUFFER_SRGB);

      window_->EndRenderFrame(imgui_enabled_);
    }
  } else {
    for (int i = 0; i < settings_.num_samples; i++) {
      cpu_tracer_.Update(scene);
    }
    write_image();
  }
}

void App::OnEvent(SDL_Event& event) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_g && event.key.keysym.mod & KMOD_ALT) {
      imgui_enabled_ = !imgui_enabled_;
      return;
    }
  }
  if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
    OnResize(glm::ivec2{event.window.data1, event.window.data2});
  }
}

}  // namespace raytrace2
