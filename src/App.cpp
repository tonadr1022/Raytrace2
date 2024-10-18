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

cpu::Camera cam;

void MakeAFinalRenderScene(cpu::Scene& scene) {
  scene = {};
  scene.materials.emplace_back(cpu::MaterialLambertian{.albedo = {0.5, 0.5, 0.5}});
  scene.hittable_list.Add(std::make_shared<cpu::Sphere>(vec3{0, -1000, 0}, 1000, 0));
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      float choose_mat = cpu::math::RandFloat();
      vec3 center{a + 0.9 * cpu::math::RandFloat(), 0.2, b + 0.9 * cpu::math::RandFloat()};
      if (glm::length(center - vec3(4, 0.2, 0)) > 0.9) {
        cpu::MaterialVariant mat;
        if (choose_mat < 0.8) {
          mat = cpu::MaterialLambertian{.albedo = cpu::math::RandVec3() * cpu::math::RandVec3()};
        } else if (choose_mat < 0.95) {
          mat = cpu::MaterialMetal{.albedo = cpu::math::RandVec3(0.5, 1),
                                   .fuzz = cpu::math::RandFloat(0, 0.5)};
        } else {
          mat = cpu::MaterialDielectric{.refraction_index = 1.5};
        }
        scene.hittable_list.Add(
            std::make_shared<cpu::Sphere>(center, vec3{0, cpu::math::RandFloat(0, 0.5), 0}, 0.2,
                                          static_cast<uint32_t>(scene.materials.size())));
        scene.materials.emplace_back(mat);
      }
    }
  }

  scene.hittable_list.Add(std::make_shared<cpu::Sphere>(
      vec3{0, 1, 0}, 1.0, static_cast<uint32_t>(scene.materials.size())));
  scene.materials.emplace_back(cpu::MaterialDielectric{.refraction_index = 1.5});

  scene.hittable_list.Add(std::make_shared<cpu::Sphere>(
      vec3{-4, 1, 0}, 1.0, static_cast<uint32_t>(scene.materials.size())));
  scene.materials.emplace_back(cpu::MaterialLambertian{.albedo = {0.4, 0.2, 0.1}});

  scene.hittable_list.Add(std::make_shared<cpu::Sphere>(
      vec3{4, 1, 0}, 1.0, static_cast<uint32_t>(scene.materials.size())));
  scene.materials.emplace_back(cpu::MaterialMetal{.albedo = {0.7, 0.6, 0.5}, .fuzz = 0.0});
}

bool imgui_enabled{true};
}  // namespace

void App::OnResize(glm::ivec2 dims) {
  viewport_dims = dims;
  output_tex = std::make_unique<gl::Texture>(gl::Tex2DCreateInfoEmpty{.dims = dims,
                                                                      .wrap_s = GL_REPEAT,
                                                                      .wrap_t = GL_REPEAT,
                                                                      .internal_format = GL_RGBA8,
                                                                      .min_filter = GL_NEAREST,
                                                                      .mag_filter = GL_NEAREST});
  cpu_tracer_.OnResize(dims);
}

void BindAndSetUniforms(cpu::Camera& cam, const cpu::Scene& scene) {
  auto shader = gl::ShaderManager::Get().GetShader("rtiow").value();
  shader.Bind();
  // TODO: make getters for camera
  shader.SetVec3("pixel00_loc", cam.pixel00_loc_);
  shader.SetVec3("pixel_delta_u", cam.pixel_delta_u_);
  shader.SetFloat("defocus_angle", cam.defocus_angle_);
  // shader.SetFloat("focus_dist", cam.focus_dist_);
  shader.SetVec2("resolution", cam.dims_);
  shader.SetFloat("rand_seed", cpu::math::RandFloat());
  shader.SetVec3("viewport_upper_left", cam.viewport_upper_left_);
  shader.SetVec3("cam_center", cam.center_);
  // shader.SetVec3("cam_lookat", cam.lookat_);
  shader.SetVec3("defocus_disk_u", cam.defocus_disk_u_);
  shader.SetVec3("defocus_disk_v", cam.defocus_disk_v_);
  shader.SetInt("num_spheres", scene.hittable_list.objects.size());
  // TODO: parameterize
  shader.SetInt("max_depth", 10);
}

void App::Run() {
  AppSettings app_settings = serialize::LoadAppSettings(GET_PATH("data/settings.json"));
  Window window{1600, 900, "raytrace_2", [this](SDL_Event& event) { OnEvent(event); }};
  gl::ShaderManager::Init();
  gl::ShaderManager::Get().AddShader(
      "textured_quad", {{GET_SHADER_PATH("textured_quad.vs.glsl"), gl::ShaderType::kVertex, {}},
                        {GET_SHADER_PATH("textured_quad.fs.glsl"), gl::ShaderType::kFragment, {}}});
  gl::ShaderManager::Get().AddShader(
      "rtiow", {{GET_SHADER_PATH("textured_quad.vs.glsl"), gl::ShaderType::kVertex, {}},
                {GET_SHADER_PATH("rtiow.fs.glsl"), gl::ShaderType::kFragment, {}}});

  CameraSettings cam_settings = serialize::LoadCameraSettings(GET_PATH("data/cam_config.json"));
  serialize::WriteCameraSettings(cam_settings, GET_PATH("data/cam_config.json"));

  Quad quad;
  quad.Init();
  // cam.SetCenter({0, 0, 1});
  // cam.SetLookAt({0, 0, 0});
  // TODO: put these params in json
  // SCENE 2
  // cam.SetFOV(20);
  // cam.SetDefocusAngle(0.5);
  // cam.SetFocusDistance(5.4);
  // cam.SetCenter({-4, 4, 1});
  // cam.SetLookAt({0, 0, -1});

  cam.SetFOV(20);
  cam.SetCenter({13, 2, 3});
  cam.SetLookAt({0, 0, 0});
  cam.SetDefocusAngle(0.6);
  cam.SetFocusDistance(10.0);

  // cam.SetCenter({-2, 2, 1});
  // cam.SetLookAt({0, 0, -1});
  // cam.SetViewUp({0, 1, 0});
  // cam.SetFOV(90);
  // cam.SetDefocusAngle(0);
  // cam.SetFocusDistance(3.4);
  cpu_tracer_.camera = &cam;
  OnResize(window.GetWindowSize());

  GLuint fbo;
  glCreateFramebuffers(1, &fbo);
  auto shader = gl::ShaderManager::Get().GetShader("textured_quad").value();
  shader.Bind();
  quad.Bind();

  uint64_t curr_time = SDL_GetPerformanceCounter();
  uint64_t prev_time = 0;
  double dt = 0;

  auto scene_opt = serialize::LoadScene(GET_PATH("data/scene2.json"));

  if (!scene_opt.has_value()) {
    exit(1);
  }
  MakeAFinalRenderScene(scene_opt.value());
  // serialize::WriteScene(scene_opt.value(), GET_PATH("data/final_render_scene_blur.json"));
  cpu::Scene& scene = scene_opt.value();
  // MakeAFinalRenderScene(scene);
  // serialize::WriteScene(scene, GET_PATH("data/final_render_book_1.json"));

  bool cpu_render = true;

  // InitGpuData(data);
  // UploadScene(data, scene);
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

    static bool rendered_once = false;
    static size_t frame_count = 0;
    frame_count++;
    if (cpu_render) {
      bool done = (rendered_once && frame_count > app_settings.num_samples);
      if (!done || !app_settings.render_once) {
        rendered_once = true;
      }
      cpu_tracer_.Update(scene);

      static bool saved = false;
      if (!saved && done && app_settings.save_after_render_once) {
        if (!std::filesystem::exists(GET_PATH("output/"))) {
          std::filesystem::create_directory(GET_PATH("output/"));
        }
        std::string out_path =
            GET_PATH("output/") + std::string("render_") + util::CurrentDateTime() + ".png";
        std::cout << "Writing image: " << out_path << '\n';
        util::WriteImage(output_tex->Id(), 4, out_path);
        saved = true;
      }
    }

    window.StartRenderFrame(imgui_enabled);

    ImGui::Begin("Settings");
    bool vsync = window.GetVsync();
    if (ImGui::Checkbox("Vsync", &vsync)) {
      window.SetVsync(vsync);
    }
    static char scene_name[100];
    static char copy_file_scene_name[100];
    ImGui::Text("Frame Count %i", static_cast<int>(frame_count));
    ImGui::InputText("##Scene Name", scene_name, 100);
    ImGui::SameLine();
    ImGui::Checkbox("CPU", &cpu_render);
    if (ImGui::Button("Load Scene")) {
      auto scene_opt = serialize::LoadScene(GET_PATH("data/") + std::string(scene_name));
      if (scene_opt.has_value()) {
        scene = scene_opt.value();
        cpu_tracer_.Reset();
      }
    }
    ImGui::InputText("##Copy File Scene Name", copy_file_scene_name, 100);
    ImGui::SameLine();
    if (ImGui::Button("Copy Scene To File")) {
      serialize::WriteScene(scene, GET_PATH("data/") + std::string(copy_file_scene_name));
    }
    ImGui::End();

    cpu_tracer_.OnImGui();

    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (cpu_render) {
      output_tex->Bind(0);
      glTextureSubImage2D(output_tex->Id(), 0, 0, 0, viewport_dims.x, viewport_dims.y, GL_RGBA,
                          GL_UNSIGNED_BYTE, cpu_tracer_.Pixels().data());
      quad.Draw();
    } else {
      // BindGPUData(data);
      // BindAndSetUniforms(cam, scene);
      quad.Draw();
    }

    window.EndRenderFrame(imgui_enabled);
  }
}

void App::OnEvent(SDL_Event& event) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_g && event.key.keysym.mod & KMOD_ALT) {
      imgui_enabled = !imgui_enabled;
      return;
    }
  }
  if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
    OnResize(glm::ivec2{event.window.data1, event.window.data2});
  }
}

}  // namespace raytrace2
