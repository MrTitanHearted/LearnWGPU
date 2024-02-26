#pragma once

#include <GLFW/glfw3.h>
#include <wgpu/wgpu.h>

WGPUSurface glfwGetWGPUSurface(GLFWwindow *window, WGPUInstance instance);