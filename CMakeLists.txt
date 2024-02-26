cmake_minimum_required(VERSION 3.20)

set(wgpu_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wgpu-cmake)

list(APPEND CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_CMAKE})

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(BUILD_SHARED_LIBS ON)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT LearnWGPU)
project(
    ${PROJECT}
    VERSION 0.1.0
    LANGUAGES CXX C
)

include_directories(include)

find_package(wgpu CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT} ${SOURCES})
target_link_libraries(${PROJECT}
    PRIVATE WebGPU::WebGPU glfw fmt::fmt imgui::imgui
)
target_include_directories(${PROJECT} PRIVATE ${Stb_INCLUDE_DIR})
