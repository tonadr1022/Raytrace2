#include "PerlinNoiseGen.hpp"

#include "cpu_raytrace/Math.hpp"

namespace raytrace2::cpu {

PerlinNoiseGen::PerlinNoiseGen(int point_count) : point_count_(point_count) { Init(); }
PerlinNoiseGen::PerlinNoiseGen() { Init(); }

float PerlinInterp(vec3 c[2][2][2], float u, float v, float w) {
  float uu = u * u * (3 - 2 * u);
  float vv = v * v * (3 - 2 * v);
  float ww = w * w * (3 - 2 * w);
  float accum = 0;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        vec3 weight_v{u - i, v - j, w - k};
        accum += (i * uu + (1 - i) * (1 - uu)) * (j * vv + (1 - j) * (1 - vv)) *
                 (k * ww + (1 - k) * (1 - ww)) * glm::dot(c[i][j][k], weight_v);
      }
    }
  }
  return accum;
}

float TrilinearInterp(float c[2][2][2], float u, float v, float w) {
  float accum = 0.0;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        accum += (i * u + (1 - i) * (1 - u)) * (j * v + (1 - j) * (1 - v)) *
                 (k * w + (1 - k) * (1 - w)) * c[i][j][k];
      }
    }
  }
  return accum;
}

void PerlinNoiseGen::Init() {
  rand_vec3_.resize(point_count_);
  for (int i = 0; i < point_count_; i++) {
    rand_vec3_[i] = glm::normalize(math::RandVec3(-1, 1));
  }

  GeneratePerm(perm_x_);
  GeneratePerm(perm_y_);
  GeneratePerm(perm_z_);
}

float PerlinNoiseGen::Turb(const vec3& p) const { return Turb(p, 7); }

float PerlinNoiseGen::Turb(const vec3& p, int depth) const {
  float accum = 0.f;
  vec3 temp_p = p;
  float weight = 1.0f;
  for (int i = 0; i < depth; i++) {
    accum += weight * Noise(temp_p);
    weight *= 0.5;
    temp_p *= 2;
  }
  return std::fabs(accum);
}

float PerlinNoiseGen::Noise(const vec3& p) const {
  float u = p.x - std::floor(p.x);
  float v = p.y - std::floor(p.y);
  float w = p.z - std::floor(p.z);
  vec3 c[2][2][2];
  // u = u * u * (3 - 2 * u);
  // v = v * v * (3 - 2 * v);
  // w = w * w * (3 - 2 * w);

  auto i = static_cast<int>(std::floor(p.x));
  auto j = static_cast<int>(std::floor(p.y));
  auto k = static_cast<int>(std::floor(p.z));

  for (int di = 0; di < 2; di++) {
    for (int dj = 0; dj < 2; dj++) {
      for (int dk = 0; dk < 2; dk++) {
        c[di][dj][dk] =
            rand_vec3_[perm_x_[(i + di) & 255] ^ perm_y_[(j + dj) & 255] ^ perm_z_[(k + dk) & 255]];
      }
    }
  }
  return PerlinInterp(c, u, v, w);
}

void PerlinNoiseGen::GeneratePerm(std::vector<int>& perm) const {
  perm.clear();
  perm.reserve(point_count_);
  for (int i = 0; i < point_count_; i++) {
    perm.emplace_back(i);
  }

  for (int i = point_count_ - 1; i > 0; i--) {
    int target = math::RandInt(0, i);
    int tmp = perm[i];
    perm[i] = perm[target];
    perm[target] = tmp;
  }
}

}  // namespace raytrace2::cpu
