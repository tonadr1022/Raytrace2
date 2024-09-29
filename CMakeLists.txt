cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/vcpkg.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(raytrace_2 VERSION 1.0)

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_compile_definitions(TRACY_ENABLE)
option(TRACY_ENABLE "" ON)

include_directories(src)
add_subdirectory(src)
