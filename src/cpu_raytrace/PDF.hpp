#pragma once

#include "Defs.hpp"
#include "cpu_raytrace/Hittable.hpp"
#include "cpu_raytrace/Math.hpp"

namespace raytrace2::cpu {

struct Hittable;

class PDF {
 public:
  virtual ~PDF() = default;
  [[nodiscard]] virtual real Value(const vec3& dir) const = 0;
  [[nodiscard]] virtual vec3 Generate() const = 0;
};

class SpherePDF : public PDF {
 public:
  SpherePDF() = default;
  [[nodiscard]] real Value(const vec3&) const override { return 1 / (4 * std::numbers::pi); }

  [[nodiscard]] vec3 Generate() const override { return math::RandUnitVec3(); }
};

class CosinePDF : public PDF {
 public:
  explicit CosinePDF(const vec3& w) : uvw_(math::GetONB(w)) {}

  [[nodiscard]] real Value(const vec3& dir) const override {
    return std::fmax(0, glm::dot(glm::normalize(dir), uvw_[2]) / std::numbers::pi);
  }

  [[nodiscard]] vec3 Generate() const override {
    return math::Transform(uvw_, math::RandomCosineDirection());
  }

 private:
  ONBVecs uvw_;
};

class HittablePDF : public PDF {
 public:
  HittablePDF(const Hittable& objects, const vec3& origin) : objects_(objects), origin_(origin) {}
  [[nodiscard]] real Value(const vec3& dir) const override {
    return objects_.PDFValue(origin_, dir);
  }
  [[nodiscard]] vec3 Generate() const override { return objects_.Random(origin_); }

 private:
  const Hittable& objects_;
  vec3 origin_;
};

class MixturePDF : public PDF {
 public:
  MixturePDF(const std::shared_ptr<PDF>& p0, const std::shared_ptr<PDF>& p1) : p_({p0, p1}) {}
  [[nodiscard]] real Value(const vec3& dir) const override {
    return 0.5 * p_[0]->Value(dir) + 0.5 * p_[1]->Value(dir);
  }

  [[nodiscard]] vec3 Generate() const override {
    if (math::RandReal() < 0.5) {
      return p_[0]->Generate();
    }
    return p_[1]->Generate();
  }

 private:
  std::array<std::shared_ptr<PDF>, 2> p_;
};

}  // namespace raytrace2::cpu
