#include "Util.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

#pragma clang diagnostic pop

namespace raytrace2::util {

nlohmann::json LoadJsonFile(const std::string& path) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    std::cerr << "Failed to open json file: " << path << '\n';
    return {};
  }
  try {
    return nlohmann::json::parse(file_stream);
  } catch (const nlohmann::json::parse_error& e) {
    return {};
  }
}

void WriteJson(nlohmann::json& obj, const std::string& path) {
  std::ofstream f(path);
  f << std::setw(2) << obj << std::endl;
}

void WriteImage(uint32_t tex, uint32_t num_channels, const std::string& out_path) {
  stbi_flip_vertically_on_write(true);
  int w, h;
  int mip_level = 0;
  glGetTextureLevelParameteriv(tex, mip_level, GL_TEXTURE_WIDTH, &w);
  glGetTextureLevelParameteriv(tex, mip_level, GL_TEXTURE_HEIGHT, &h);
  std::vector<uint8_t> pixels(static_cast<size_t>(w) * h * num_channels);
  glGetTextureImage(tex, mip_level, num_channels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE,
                    sizeof(uint8_t) * pixels.size(), pixels.data());
  // Apply gamma correction (inverse of 2.2 gamma correction)
  for (unsigned char& pixel : pixels) {
    float value = pixel / 255.0f;
    value = pow(value, 1.0f / 2.2f);  // Convert sRGB to linear
    pixel = static_cast<uint8_t>(value * 255.0f);
  }
  stbi_write_png(out_path.c_str(), w, h, num_channels, pixels.data(), w * num_channels);
}

std::string CurrentDateTime() {
  time_t now = time(nullptr);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

  return buf;
}
}  // namespace raytrace2::util
