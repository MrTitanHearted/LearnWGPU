cmake_minimum_required(VERSION 3.20)

set(wgpu_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wgpu-cmake)

list(APPEND CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_CMAKE})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT LearnWGPU)
project(
    ${PROJECT}
    VERSION 0.1.0
    LANGUAGES CXX
)
source_group("Assets" FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets)

include_directories(include)

find_package(wgpu CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "include/*.hpp")
add_executable(${PROJECT} ${SOURCES})
target_link_libraries(${PROJECT}
    PRIVATE WebGPU::WebGPU glfw fmt::fmt imgui::imgui assimp::assimp
)
target_include_directories(${PROJECT} PRIVATE ${Stb_INCLUDE_DIR})
