#include "App.hpp"

#include <SDL_events.h>
#include <imgui.h>

#include <cstddef>

#include "Window.hpp"
#include "gl/Texture.hpp"

namespace raytrace2 {

void App::Run() {
  Window window{false, "raytrace_2", [this](SDL_Event& event) { OnEvent(event); }};
  bool imgui_enabled{true};
  auto dims = window.GetWindowSize();
  gl::Texture tex{gl::Tex2DCreateInfoEmpty{.dims = dims,
                                           .internal_format = GL_RGB8,
                                           .wrap_s = GL_REPEAT,
                                           .wrap_t = GL_REPEAT,
                                           .min_filter = GL_NEAREST,
                                           .mag_filter = GL_NEAREST}};
  std::vector<uint8_t> pixels(static_cast<size_t>(dims.x * dims.y * 3));
  for (int i = 0; i < dims.x * dims.y * 3; i += 4) {
    pixels[i] = 255;
    pixels[i + 1] = 0;
    pixels[i + 2] = 255;
  }
  glTextureSubImage2D(tex.Id(), 0, 0, 0, dims.x, dims.y, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

  GLuint fbo, rbo;
  glCreateFramebuffers(1, &fbo);
  glCreateRenderbuffers(1, &rbo);
  glNamedRenderbufferStorage(rbo, GL_DEPTH_COMPONENT24, dims.x, dims.y);
  glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
  if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "framebuffer incomplete\n";
    exit(1);
  }

  while (!window.ShouldClose()) {
    window.PollEvents();
    window.StartRenderFrame(imgui_enabled);
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    window.EndRenderFrame(imgui_enabled);
  }
}

void App::OnEvent(SDL_Event&) {}

}  // namespace raytrace2
