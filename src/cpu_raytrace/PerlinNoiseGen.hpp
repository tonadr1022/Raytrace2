#pragma once

#include "Defs.hpp"
namespace raytrace2::cpu {

class PerlinNoiseGen {
 public:
  explicit PerlinNoiseGen(int point_count);
  PerlinNoiseGen();
  [[nodiscard]] float Noise(const vec3& p) const;
  [[nodiscard]] float Turb(const vec3& p, int depth) const;
  [[nodiscard]] float Turb(const vec3& p) const;

 private:
  std::vector<int> perm_x_;
  std::vector<int> perm_y_;
  std::vector<int> perm_z_;
  std::vector<vec3> rand_vec3_;
  // std::vector<float> randfloat_;
  int point_count_{256};

  void GeneratePerm(std::vector<int>& perm) const;
  void Init();
};

}  // namespace raytrace2::cpu
