#pragma once

#include <glm/fwd.hpp>
#define DOUBLE
#ifdef DOUBLE
using real = double;
constexpr const real kInfinity = std::numeric_limits<real>::max();
using color = glm::u8vec4;
using vec3 = glm::dvec3;
using vec2 = glm::dvec2;
using vec4 = glm::dvec4;
using mat4 = glm::dmat4;
using mat3 = glm::dmat3;
using quat = glm::dquat;
#else
using real = float;
constexpr const real kInfinity = std::numeric_limits<real>::max();
using color = glm::u8vec4;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using mat3 = glm::mat3;
using quat = glm::quat;
#endif
