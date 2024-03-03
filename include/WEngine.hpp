#pragma once

#include <WInclude.hpp>
#include <WTexture.hpp>

class WEngine {
   public:
    WEngine(const WEngine &) = delete;
    WEngine &operator=(const WEngine &) = delete;

    static void Init();
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

    WTexture depthTexture;

    WEngine();
    ~WEngine();

    void presentFrame(std::function<void(WGPUTextureView)> frame);

    static void handleGlfwKey(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void resizeGlfwFramebuffer(GLFWwindow *window, int width, int height);
    static WGPUShaderModule shaderFromFile(WGPUDevice device, const char *path);
};