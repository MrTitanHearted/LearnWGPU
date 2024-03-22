#pragma once

#include <WInclude.hpp>
#include <WCamera.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.hpp>

class WEngine {
   public:
    WEngine(const WEngine &) = delete;
    WEngine &operator=(const WEngine &) = delete;

    static void Initialize();
    static void Shutdown();

    static WEngine &GetInstance();

    void run();

   private:
    static WEngine *engine;

    std::string title = "LearnWGPU";
    uint32_t width = 1200;
    uint32_t height = 1000;
    GLFWwindow *window;

    WGPUInstance instance;
    WGPUSurface surface;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurfaceConfiguration config;
    WGPULimits limits;
    WTexture depthTexture;

    WCameraManager camera{600, 500};

    glm::mat4 modelData{1.0f};
    float scale = 1.0f / 20.f;

    float dt;

    WEngine();
    ~WEngine();

    void initImGui();
    void shutdownImGui();
    void updateImGui(WGPURenderPassEncoder encoder);

    void presentFrame(std::function<void(WGPUTextureView)> frame);
    void setupLogging(WGPULogLevel level = WGPULogLevel_Warn) const;
    void printWGPUReport() const;

    static void glfwKeyCallback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
    static void glfwFramebuffersizeCallback(GLFWwindow *window, int32_t width, int32_t height);
    static void glfwCursorPosCallback(GLFWwindow *window, double x, double y);
    static void glfwScrollCallabck(GLFWwindow *window, double x, double y);

    static void wgpuLogCallback(WGPULogLevel level, const char *message, void *userdata);

    static WGPUShaderModule shaderFromWgslFile(WGPUDevice device, std::string path);
};
