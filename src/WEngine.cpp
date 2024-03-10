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

    static WVertex New(float x, float y, float z, float r, float g, float b, float u, float v) {
        return WVertex{.pos = glm::vec3(x, y, z), .color = glm::vec3(r, g, b), .uv = glm::vec2(u, v)};
    }
    static WVertexLayout desc();
};

WVertexLayout WVertex::desc() {
    return WVertexLayout{sizeof(WVertex)}
        .addAttributes(WGPUVertexFormat_Float32x3, offsetof(WVertex, pos), 0)
        .addAttributes(WGPUVertexFormat_Float32x3, offsetof(WVertex, color), 1)
        .addAttributes(WGPUVertexFormat_Float32x2, offsetof(WVertex, uv), 2);
}

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
    std::vector<WVertex> triangleVertices{
        WVertex::New(-0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0),
        WVertex::New(0.0, 0.5, 0.5, 0.0, 1.0, 0.0, 0.5, 1.0),
        WVertex::New(0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0),
    };
    std::vector<uint32_t> triangleIndices{0, 1, 2};
    std::vector<WVertex> quadVertices{
        WVertex::New(-0.5, -0.5, 0.8, 1.0, 0.0, 0.0, 0.0, 0.0),
        WVertex::New(-0.5, 0.5, 0.8, 0.0, 1.0, 0.0, 0.0, 1.0),
        WVertex::New(0.5, 0.5, 0.8, 0.0, 0.0, 1.0, 1.0, 1.0),
        WVertex::New(0.5, -0.5, 0.8, 1.0, 1.0, 0.0, 1.0, 0.0),
    };
    std::vector<uint32_t> quadIndices{0, 1, 2, 0, 2, 3};
    WRenderBuffer triangle = WRenderBufferBuilder{}
                                 .setVertices(triangleVertices)
                                 .setIndices(triangleIndices)
                                 .build(device);
    WRenderBuffer quad = WRenderBufferBuilder{}
                             .setVertices(quadVertices)
                             .setIndices(quadIndices)
                             .build(device);

    glm::mat4 firstOne{1.0f};
    glm::mat4 secondOne{1.0};

    firstOne = glm::scale(firstOne, glm::vec3(1.0 / 2.0));
    firstOne = glm::translate(firstOne, glm::vec3(0.3, 0.2, 0.0));
    secondOne = glm::scale(secondOne, glm::vec3(1.0 / 3.0));
    secondOne = glm::translate(secondOne, glm::vec3(-0.3, -0.2, 1.0));
    uint32_t offsetAlignment = limits.minUniformBufferOffsetAlignment;
    WUniformBuffer uniform{device, &firstOne, 2 * offsetAlignment};
    uniform.update(queue, &firstOne, 0, sizeof(firstOne));
    uniform.update(queue, &secondOne, offsetAlignment, sizeof(secondOne));

    fmt::println("maxDynamicUniformBuffersPerPipelineLayout: {}", limits.maxDynamicUniformBuffersPerPipelineLayout);

    WGPUSampler sampler = WSamplerBuilder{}.build(device);
    WTexture texture = WTextureBuilder::fromFileAsRgba8(device, "assets/textures/container.jpg");
    WGPUBindGroupLayout bindGroupLayout = WBindGroupLayoutBuilder{}
                                              .addBindingSampler(0)
                                              .addBindingTexture(1)
                                              .addBindingDynamicUniform(2)
                                              .build(device);
    WGPUBindGroup bindGroup = WBindGroupBuilder{}
                                  .addBindingSampler(0, sampler)
                                  .addBindingTexture(1, texture)
                                  .addBindingUniform(2, uniform, sizeof(firstOne))
                                  .build(device, bindGroupLayout);
    WGPUPipelineLayout pipelineLayout = WPipelineLayoutBuilder{}
                                            .addBindGroupLayout(bindGroupLayout)
                                            .build(device);
    WGPURenderPipeline pipeline = WRenderPipelineBuilder{}
                                      .setVertexState(shader, "vs_main")
                                      .setFragmentState(shader, "fs_main")
                                      .addVertexBufferLayout(WVertex::desc())
                                      .addColorTarget(config.format)
                                      .setDefaultDepthState()
                                      .build(device, pipelineLayout);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        presentFrame([&](WGPUTextureView frame) {
            WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
            std::vector<WGPUCommandBuffer> commandBuffers{};

            WGPURenderPassEncoder renderPassEncoder = WRenderPassBuilder{}
                                                          .addColorAttachment(frame, WGPUColor{0.2, 0.3, 0.3, 1.0})
                                                          .setDepthAttachment(depthTexture)
                                                          .build(commandEncoder);
            wgpuRenderPassEncoderSetPipeline(renderPassEncoder, pipeline);

            wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, triangle.vertex, 0, triangle.verticesSize);
            wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, triangle.index, WGPUIndexFormat_Uint32, 0, triangle.indicesSize);

            uint32_t dynamicOffset = 0;
            wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, bindGroup, 1, &dynamicOffset);
            wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, triangle.indicesCount, 1, 0, 0, 0);

            wgpuRenderPassEncoderSetVertexBuffer(renderPassEncoder, 0, quad.vertex, 0, quad.verticesSize);
            wgpuRenderPassEncoderSetIndexBuffer(renderPassEncoder, quad.index, WGPUIndexFormat_Uint32, 0, quad.indicesSize);

            dynamicOffset = offsetAlignment;
            wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, bindGroup, 1, &dynamicOffset);
            wgpuRenderPassEncoderDrawIndexed(renderPassEncoder, quad.indicesCount, 1, 0, 0, 0);

            wgpuRenderPassEncoderEnd(renderPassEncoder);
            commandBuffers.push_back(wgpuCommandEncoderFinish(commandEncoder, nullptr));

            wgpuQueueSubmit(queue, commandBuffers.size(), commandBuffers.data());
        });
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

    depthTexture = WTexture::GetDepthTexture(device, WGPUExtent3D{width, height, 1});

    WGPUSupportedLimits supportedLimits{};
    wgpuDeviceGetLimits(device, &supportedLimits);
    limits = supportedLimits.limits;
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

void WEngine::presentFrame(std::function<void(WGPUTextureView)> frame) {
    bool skip = false;
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

                depthTexture = WTexture::GetDepthTexture(device, WGPUExtent3D{(uint32_t)width, (uint32_t)height, 1});
            }
            std::cout << "Resizing in the main function!" << std::endl;
            skip = true;
        }
        case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
        case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
        case WGPUSurfaceGetCurrentTextureStatus_Force32:
            throw std::runtime_error(fmt::format("get_current_texture status={}", (uint32_t)surfaceTexture.status));
    }

    if (!skip) {
        WGPUTextureView target = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
        frame(target);
        wgpuSurfacePresent(surface);

        wgpuTextureViewRelease(target);
        wgpuTextureRelease(surfaceTexture.texture);
    }
}

void WEngine::handleGlfwKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // WEngine &engine = *(WEngine *)glfwGetWindowUserPointer(window);
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

    engine.depthTexture = WTexture::GetDepthTexture(engine.device, WGPUExtent3D{(uint32_t)width, (uint32_t)height, 1});
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
