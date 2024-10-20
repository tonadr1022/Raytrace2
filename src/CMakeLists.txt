project(raytrace_2)

set(SOURCES
    main.cpp
    App.cpp
    Window.cpp
    EAssert.cpp
    pch.cpp
    Serialize.cpp
    Util.cpp
    gl/Texture.cpp
    gl/Shader.cpp
    gl/ShaderManager.cpp
    gl/VertexArray.cpp
    cpu_raytrace/Sphere.cpp
    cpu_raytrace/Interval.cpp
    cpu_raytrace/RayTracer.cpp
    cpu_raytrace/Material.cpp
    cpu_raytrace/PerlinNoiseGen.cpp
    cpu_raytrace/Scene.cpp
    cpu_raytrace/Texture.cpp
    cpu_raytrace/BVH.cpp
    cpu_raytrace/HittableList.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -pedantic)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    endif()
endif()

add_compile_definitions(SRC_PATH="${CMAKE_SOURCE_DIR}")
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.hpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_HOME_DIRECTORY}/dep)


# For std::execution::par
find_package(TBB REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)

target_link_libraries(${PROJECT_NAME} PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    imgui::imgui
    GLEW::GLEW
    glm::glm
    nlohmann_json::nlohmann_json
    Tracy::TracyClient
)
