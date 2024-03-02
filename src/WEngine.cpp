#include <WEngine.hpp>

#include <WRenderBuffer.hpp>
#include <WRenderPass.hpp>
#include <WPipeline.hpp>
#include <WBindings.hpp>
#include <WSampler.hpp>
#include <WTexture.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

struct WVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;

    static WVertexLayout desc();
};

WEngine *WEngine::engine = nullptr;

void WEngine::Init() {
    if (engine == nullptr) {
        glfwInit();
        engine = new WEngine{};
    }
}

void WEngine::Shutdown() {
    if (engine != nullptr) {
        delete engine;
        engine = nullptr;
        glfwTerminate();
    }
}

WEngine &WEngine::GetInstance() {
    if (engine == nullptr) {
        throw std::runtime_error("You should first call 'Init()' method!");
    }
    return *engine;
}

void WEngine::run() {
    WGPUShaderModule shader = shaderFromFile(device, "assets/shaders/shader.wgsl");
    std::vector<WVertex> vertices{
        WVertex{.pos = glm::vec3(-0.5, -0.5, 0.0), .color = glm::vec3(1.0, 0.0, 0.0), .uv = glm::vec2(0.0, 0.0)},
        WVertex{.pos = glm::vec3(0.0, 0.5, 0.0), .color = glm::vec3(0.0, 1.0, 0.0), .uv = glm::vec2(0.5, 1.0)},
        WVertex{.pos = glm::vec3(0.5, -0.5, 0.0), .color = glm::vec3(0.0, 0.0, 1.0), .uv = glm::vec2(1.0, 0.0)},
    };
    std::vector<uint32_t> indices{0, 1, 2};
    WRenderBuffer renderBuffer = WRenderBufferBuilder{}
                                     .setVertices(vertices)
                                     .setIndices(indices)
                                     .build(device);
    WGPUSampler sampler = WSamplerBuilder{}.build(device);
    WGPUTexture texture = WTextureBuilder::fromFileAsRgba8(device, "assets/textures/awesomeface.png");
    WGPUTextureView textureView = wgpuTextureCreateView(texture, nullptr);
    WGPUBindGroupLayout bindGroupLayout = WBindGroupLayoutBuilder{}
                                              .addBindingSampler(0)
                                              .addBindingTexture(1)
                                              .build(device);
    WGPUBindGroup bindGroup = WBindGroupBuilder{}
                                  .addBindingSampler(0, sampler)
                                  .addBindingTexture(1, textureView)
                                  .build(device, bindGroupLayout);
    WGPUPipelineLayout pipelineLayout = WPipelineLayoutBuilder{}
                                            .addBindGroupLayout(bindGroupLayout)
                                            .build(device);
    WGPURenderPipeline pipeline = WRenderPipelineBuilder{}
                                      .setVertexState(shader, "vs_main")
                                      .setFragmentState(shader, "fs_main")
                                      .addVertexBufferLayout(WVertex::desc())
                                      .addColorTarget(config.format)
                                      .build(device, pipelineLayout);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
        switch (surfaceTexture.status) {
            case WGPUSurfaceGetCurrentTextureStatus_Success:
                break;
            case WGPUSurfaceGetCurrentTextureStatus_Timeout:
            case WGPUSurfaceGetCurrentTextureStatus_Outdated:
            case WGPUSurfaceGetCurrentTextureStatus_Lost: {
                if (surfaceTexture.texture != nullptr) {
                    wgpuTextureRelease(surfaceTexture.texture);
                }
                int width, height;
                glfwGetWindowSize(window, &width, &height);
                if (width != 0 && height != 0) {
                    config.width = width;
                    config.height = height;
                    wgpuSurfaceConfigure(surface, &config);
                }
                std::cout << "Resizing in the main function!" << std::endl;
                continue;
            }
            case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
            case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
            case WGPUSurfaceGetCurrentTextureStatus_Force32:
                throw std::runtime_error(fmt::format("get_current_texture status={}", (uint32_t)surfaceTexture.status));
        }
        WGPUTextureView frame = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
        WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(device, nullptr);

        WGPURenderPassEncoder renderPassEncoder = WRenderPassBuilder{}
                                                      .addColorAttachment(frame, WGPUColor{0.2, 0.3, 0.3, 1.0})
                                                      .build(commandEncoder);

        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, pipeline);
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, bindGroup, 0, nullptr);
        renderBuffer.render(renderPassEncoder);
        wgpuRenderPassEncoderEnd(renderPassEncoder);

        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, nullptr);
        wgpuQueueSubmit(queue, 1, &commandBuffer);
        wgpuSurfacePresent(surface);
    }
}

WEngine::WEngine() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, handleGlfwKey);
    glfwSetFramebufferSizeCallback(window, resizeGlfwFramebuffer);

    instance = wgpuCreateInstance(nullptr);
    surface = glfwGetWGPUSurface(window, instance);
    WGPURequestAdapterOptions adapterOpts{.compatibleSurface = surface};
    wgpuInstanceRequestAdapter(
        instance,
        &adapterOpts,
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char *message, void *userdata) {
            if (status == WGPURequestAdapterStatus_Success) {
                WEngine &engine = *(WEngine *)userdata;
                engine.adapter = adapter;
            } else {
                throw std::runtime_error(fmt::format("Failed to request an adapter!: {}", message));
            }
        },
        this);
    wgpuAdapterRequestDevice(
        adapter,
        nullptr,
        [](WGPURequestDeviceStatus status, WGPUDevice device, const char *message, void *userdata) {
            if (status == WGPURequestDeviceStatus_Success) {
                WEngine &engine = *(WEngine *)userdata;
                engine.device = device;
            } else {
                throw std::runtime_error(fmt::format("Failed to request a device!: {}", message));
            }
        },
        this);
    queue = wgpuDeviceGetQueue(device);
    WGPUSurfaceCapabilities caps;
    wgpuSurfaceGetCapabilities(surface, adapter, &caps);
    config = WGPUSurfaceConfiguration{
        .device = device,
        .format = WGPUTextureFormat_RGBA8UnormSrgb,
        .usage = WGPUTextureUsage_RenderAttachment,
        .viewFormatCount = 0,
        .viewFormats = nullptr,
        .alphaMode = caps.alphaModes[0],
        .width = width,
        .height = height,
        .presentMode = WGPUPresentMode_Fifo,
    };
#ifdef WENGINE_PLATFORM_WINDOWS
    config.format = WGPUTextureFormat_RGBA8Unorm;
#endif
    for (int i = 0; i < caps.presentModeCount; i++) {
        if (WGPUPresentMode_Mailbox == caps.presentModes[i]) {
            config.presentMode = WGPUPresentMode_Mailbox;
            break;
        }
    }
    wgpuSurfaceConfigure(surface, &config);
    wgpuSurfaceCapabilitiesFreeMembers(caps);
}

WEngine::~WEngine() {
    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);

    glfwDestroyWindow(window);
    window = nullptr;
}

void WEngine::handleGlfwKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
    WEngine &engine = *(WEngine *)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void WEngine::resizeGlfwFramebuffer(GLFWwindow *window, int width, int height) {
    if (width == 0 || height == 0) {
        return;
    }

    WEngine &engine = *(WEngine *)glfwGetWindowUserPointer(window);
    engine.width = width;
    engine.height = height;

    engine.config.width = engine.width;
    engine.config.height = engine.height;

    wgpuSurfaceConfigure(engine.surface, &engine.config);
}

WGPUShaderModule WEngine::shaderFromFile(WGPUDevice device, const char *path) {
    std::ifstream file{path};

    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Can not open shader in this path: {}", path));
    }

    std::stringstream shaderStream;
    shaderStream << file.rdbuf();
    file.close();
    std::string shaderCode = shaderStream.str();

    WGPUShaderModuleWGSLDescriptor wgslDesc{
        .chain = WGPUChainedStruct{
            .sType = WGPUSType_ShaderModuleWGSLDescriptor,
        },
        .code = shaderCode.c_str(),
    };
    WGPUShaderModuleDescriptor shaderDesc{
        .nextInChain = (const WGPUChainedStruct *)&wgslDesc,
        .label = path,
    };
    WGPUShaderModule shader = wgpuDeviceCreateShaderModule(device, &shaderDesc);
    return shader;
}

WVertexLayout WVertex::desc() {
    return WVertexLayout{sizeof(WVertex)}
        .addAttributes(WGPUVertexFormat_Float32x3, offsetof(WVertex, pos), 0)
        .addAttributes(WGPUVertexFormat_Float32x3, offsetof(WVertex, color), 1)
        .addAttributes(WGPUVertexFormat_Float32x2, offsetof(WVertex, uv), 2);
}
