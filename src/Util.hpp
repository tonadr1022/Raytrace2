#pragma once

#include <nlohmann/json_fwd.hpp>

#include "Defs.hpp"

namespace raytrace2::util {
nlohmann::json LoadJsonFile(const std::string& path);
void WriteJson(nlohmann::json& obj, const std::string& path);
void WriteImage(uint32_t tex, uint32_t num_channels, const std::string& out_path);
std::string CurrentDateTime();
void PrintMatrix(mat4& mat);

}  // namespace raytrace2::util
