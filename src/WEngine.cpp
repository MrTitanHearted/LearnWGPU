#include <WEngine.hpp>

#include <WRenderBuffer.hpp>
#include <WPipeline.hpp>
#include <WBaseVertex.hpp>
#include <WBindings.hpp>
#include <WSampler.hpp>
#include <WTexture.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

struct Vertex : WBaseVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
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
    Vertex::addVertexAttribute(WGPUVertexFormat_Float32x3, offsetof(Vertex, position), 0);
    Vertex::addVertexAttribute(WGPUVertexFormat_Float32x3, offsetof(Vertex, normal), 1);
    Vertex::addVertexAttribute(WGPUVertexFormat_Float32x2, offsetof(Vertex, uv), 2);
    Vertex::setVertexStride(sizeof(Vertex));

    WGPUShaderModule shader = shaderFromFile(device, "assets/shaders/shader.wgsl");
    std::vector<Vertex> vertices{
        Vertex{.position = glm::vec3(-0.5, -0.5, 0.0), .uv = glm::vec2(0.0, 0.0)},
        Vertex{.position = glm::vec3(-0.5, 0.5, 0.0), .uv = glm::vec2(0.0, 1.0)},
        Vertex{.position = glm::vec3(0.5, 0.5, 0.0), .uv = glm::vec2(1.0, 1.0)},
        Vertex{.position = glm::vec3(0.5, -0.5, 0.0), .uv = glm::vec2(1.0, 0.0)},
    };
    std::vector<uint32_t> indices{0, 1, 2, 0, 2, 3};
    WRenderBuffer renderBuffer = WRenderBufferBuilder{}
                                     .setVertices(vertices)
                                     .setIndices(indices)
                                     .build(device);

    WUniformBuffer uniformBuffer{device, &model, sizeof(model)};

    WGPUSampler sampler = WSamplerBuilder{}.build(device);
    WGPUTexture texture1 = WTextureBuilder{}.fromFileAsRgba8(device, "assets/textures/wall.jpg");
    WGPUTextureView texture1View = wgpuTextureCreateView(texture1, nullptr);
    WGPUTexture texture2 = WTextureBuilder{}.fromFileAsRgba8(device, "assets/textures/awesomeface.png");
    WGPUTextureView texture2View = wgpuTextureCreateView(texture2, nullptr);

    WGPUBindGroupLayout bindGroupLayout =
        WBindGroupLayoutBuilder{}
            .addBindingUniform(0)
            .addBindingSampler(1)
            .addBindingTexture(2)
            .addBindingTexture(3)
            .build(device);
    WGPUBindGroup bindGroup =
        WBindGroupBuilder{}
            .addBindingUniform(0, uniformBuffer.buffer, uniformBuffer.size)
            .addBindingSampler(1, sampler)
            .addBindingTexture(2, texture1View)
            .addBindingTexture(3, texture2View)
            .build(device, bindGroupLayout);

    WGPUPipelineLayout layout = WPipelineLayoutBuilder{}
                                    .addBindGroupLayout(bindGroupLayout)
                                    .build(device);
    WGPURenderPipeline pipeline = WRenderPipelineBuilder{}
                                      .setVertexState(shader, "vs_main")
                                      .setFragmentState(shader, "fs_main")
                                      .addColorTarget(config.format)
                                      .addVertexBufferLayout(Vertex::desc())
                                      .build(device, layout);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        model = glm::rotate(model, glm::radians(0.01f), glm::vec3(0.0, 0.0, 1.0));

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

        WGPURenderPassColorAttachment colorAttachment{
            .view = frame,
            .loadOp = WGPULoadOp_Clear,
            .storeOp = WGPUStoreOp_Store,
            .clearValue = WGPUColor{0.2, 0.3, 0.3, 1.0},
        };
        WGPURenderPassDescriptor renderPassDesc{
            .label = "LearnWGPU main render pass",
            .colorAttachmentCount = 1,
            .colorAttachments = &colorAttachment,
        };
        WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDesc);

        uniformBuffer.update(queue, &model);

        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, pipeline);
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, 0, bindGroup, 0, nullptr);

        renderBuffer.render(renderPassEncoder);

        wgpuRenderPassEncoderEnd(renderPassEncoder);

        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, nullptr);
        wgpuQueueSubmit(queue, 1, &commandBuffer);
        wgpuSurfacePresent(surface);

        wgpuCommandBufferRelease(commandBuffer);
        wgpuCommandEncoderRelease(commandEncoder);
        wgpuTextureViewRelease(frame);
        wgpuTextureRelease(surfaceTexture.texture);
    }

    wgpuBufferRelease(renderBuffer.vertex);
    wgpuBufferRelease(renderBuffer.index);
    wgpuBufferRelease(uniformBuffer.buffer);
    wgpuSamplerRelease(sampler);
    wgpuTextureViewRelease(texture1View);
    wgpuTextureRelease(texture1);
    wgpuTextureViewRelease(texture2View);
    wgpuTextureRelease(texture2);
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
        .format = caps.formats[1],
        .usage = WGPUTextureUsage_RenderAttachment,
        .viewFormatCount = 0,
        .viewFormats = nullptr,
        .alphaMode = caps.alphaModes[0],
        .width = width,
        .height = height,
        .presentMode = WGPUPresentMode_Mailbox,
    };
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