#include "Util.hpp"

#include <cstddef>
#include <fstream>
#include <glm/mat4x4.hpp>
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

void WriteImage(const std::vector<vec3>& pixels, int width, int height, const std::string& out_path,
                bool png) {
  auto to_color = [](vec3 col) {
    // Apply gamma correction (gamma 2.0)
    col = vec3{std::sqrt(col.x), std::sqrt(col.y), std::sqrt(col.z)};

    // Scale to [0, 255] and then clamp
    return glm::ivec3{static_cast<int>(std::clamp(col.x * 255.999, 0.0, 255.0)),
                      static_cast<int>(std::clamp(col.y * 255.999, 0.0, 255.0)),
                      static_cast<int>(std::clamp(col.z * 255.999, 0.0, 255.0))};
  };
  if (png) {
    stbi_flip_vertically_on_write(true);
    std::vector<unsigned char> data(static_cast<size_t>(width * height * 3));
    for (int h = height - 1; h >= 0; h--) {
      for (int w = 0; w < width; w++) {
        auto col = to_color(pixels[h * width + w]);
        int idx = (h * width + w) * 3;
        data[idx] = static_cast<unsigned char>(col.x);
        data[idx + 1] = static_cast<unsigned char>(col.y);
        data[idx + 2] = static_cast<unsigned char>(col.z);
      }
    }
    stbi_write_png((out_path + ".png").c_str(), width, height, 3, data.data(),
                   width * sizeof(unsigned char) * 3);
  } else {
    std::ofstream f(out_path + ".ppm");
    f << "P3\n" << width << ' ' << height << "\n255\n";
    for (int h = height - 1; h >= 0; h--) {
      for (int w = 0; w < width; w++) {
        auto col = to_color(pixels[h * width + w]);
        f << col.x << ' ' << col.y << ' ' << col.z << '\n';
      }
    }
  }
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

void PrintMatrix(mat4& matrix) {
  std::cout << std::fixed << std::setprecision(4);  // Optional: format for consistent precision
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      std::cout << std::setw(10) << matrix[i][j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

}  // namespace raytrace2::util
