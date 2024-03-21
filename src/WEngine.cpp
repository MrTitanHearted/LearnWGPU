#include <WEngine.hpp>

#include <WUtils.hpp>

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
    return WVertexLayout::New(sizeof(WVertex))
        .addAttribute(WGPUVertexFormat_Float32x3, offsetof(WVertex, pos), 0)
        .addAttribute(WGPUVertexFormat_Float32x3, offsetof(WVertex, color), 1)
        .addAttribute(WGPUVertexFormat_Float32x2, offsetof(WVertex, uv), 2);
}

WEngine *WEngine::engine = nullptr;

void WEngine::Initialize() {
    if (engine == nullptr) {
        glfwInit();

        engine = new WEngine();
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
        throw std::exception("[WEngine]::[ERROR]: You should call 'Initialize()' method first!");
    }

    return *engine;
}

void WEngine::run() {
    WGPUShaderModule shader = shaderFromWgslFile(device, "assets/shaders/shader.wgsl");

        std::vector<WVertex> vertices{
            WVertex::New(-0.5, -0.5, 0.8, 1.0, 0.0, 0.0, 0.0, 0.0),
            WVertex::New(-0.5, 0.5, 0.8, 0.0, 1.0, 0.0, 0.0, 1.0),
            WVertex::New(0.5, 0.5, 0.8, 0.0, 0.0, 1.0, 1.0, 1.0),
            WVertex::New(0.5, -0.5, 0.8, 1.0, 1.0, 0.0, 1.0, 0.0),
        };
        std::vector<uint32_t> indices{0, 1, 2, 0, 2, 3};

    WRenderBuffer renderBuffer =
        WRenderBufferBuilder::New()
            .setVertices(vertices)
            .setIndices(indices)
            .build(device);

    WGPUSampler sampler = WSamplerBuilder::New().build(device);
    WTexture texture = WTexture::fromFileAsRgba8(device, "assets/textures/awesomeface.png");
    glm::mat4 uniformData{1.0};
    WUniformBuffer uniformBuffer = WUniformBuffer::New(device, &uniformData, sizeof(uniformData));

    WBindGroup bindGroup = 
        WBindGroupBuilder::New()
        .addBindingSampler(0, sampler)
        .addBindingTexture(1, texture)
        .addBindingUniform(2, uniformBuffer)
        .build(device);

    WRenderPipeline pipeline =
        WRenderPipelineBuilder::New()
            .addBindGroupLayout(bindGroup)
            .setVertexState(shader)
            .setFragmentState(shader)
            .addColorTarget(config.format)
            .addVertexBufferLayout(WVertex::desc())
            .setDefaultDepthState()
            .build(device);

    WRenderBundle renderBundle =
        WRenderBundleBuilder::New()
            .addBindGroup(bindGroup)
            .setRenderPipeline(pipeline)
            .setRenderBuffer(renderBuffer)
            .addColorFormat(config.format)
            .setDefaultDepthFormat()
            .build(device);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        presentFrame([&](WGPUTextureView frame) {
            WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
            std::vector<WGPUCommandBuffer> commandBuffers{};

            WGPURenderPassEncoder encoder =
                WRenderPassBuilder::New()
                    .addColorTarget(WColorAttachment::New(frame).setClearColor(0.2, 0.3, 0.3, 1.0))
                    .setDepthAttachment(WDepthStencilAttachment::New(depthTexture))
                    .build(commandEncoder);

            renderBundle.render(encoder);

            wgpuRenderPassEncoderEnd(encoder);

            commandBuffers.push_back(wgpuCommandEncoderFinish(commandEncoder, nullptr));

            wgpuQueueSubmit(queue, commandBuffers.size(), commandBuffers.data());

            for (const WGPUCommandBuffer &commandBuffer : commandBuffers) {
                wgpuCommandBufferRelease(commandBuffer);
            }
            wgpuCommandEncoderRelease(commandEncoder);
        });
    }
}

WEngine::WEngine() {
    setupLogging();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetFramebufferSizeCallback(window, glfwFramebuffersizeCallback);

    WGPUInstanceExtras instanceExtras{
        .chain = WGPUChainedStruct{
            .sType = (WGPUSType)WGPUSType_InstanceExtras,
        },
        .backends = WGPUInstanceBackend_Primary,
    };
    WGPUInstanceDescriptor instanceDescriptor{
        .nextInChain = (WGPUChainedStruct *)&instanceExtras,
    };
#ifdef WENGINE_PLATFORM_WINDOWS
    instanceExtras.backends = WGPUInstanceBackend_DX12;
#endif
    instance = wgpuCreateInstance(&instanceDescriptor);

    surface = glfwGetWGPUSurface(window, instance);
    WGPURequestAdapterOptions adapterOptions{.compatibleSurface = surface};
    wgpuInstanceRequestAdapter(
        instance,
        &adapterOptions,
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char *message, void *userdata) {
            switch (status) {
                case WGPURequestAdapterStatus_Success: {
                    WGPUAdapter *engineAdapter = (WGPUAdapter *)userdata;
                    *engineAdapter = adapter;
                } break;
                default: {
                    throw std::exception(fmt::format("[WEngine]::[ERROR]: Failed to request adapter!: '{}'", message).c_str());
                } break;
            }
        },
        &this->adapter);
    wgpuAdapterRequestDevice(
        adapter,
        nullptr,
        [](WGPURequestDeviceStatus status, WGPUDevice device, const char *message, void *userdata) {
            switch (status) {
                case WGPURequestDeviceStatus_Success: {
                    WGPUDevice *engineDevice = (WGPUDevice *)userdata;
                    *engineDevice = device;
                } break;
                default: {
                    throw std::exception(fmt::format("[WEngine]::[ERROR]: Failed to request device!: '{}'", message).c_str());
                } break;
            }
        },
        &this->device);
    queue = wgpuDeviceGetQueue(device);
    WGPUSurfaceCapabilities caps{};
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
        .presentMode = caps.presentModes[0],
    };
#ifdef WENGINE_PLATFORM_WINDOWS
    config.format = WGPUTextureFormat_RGBA8Unorm;
#endif
    for (uint32_t i = 0; i < caps.presentModeCount; i++) {
        if (WGPUPresentMode_Mailbox == caps.presentModes[i]) {
            config.presentMode = WGPUPresentMode_Mailbox;
            break;
        }
    }
    wgpuSurfaceConfigure(surface, &config);
    wgpuSurfaceCapabilitiesFreeMembers(caps);

    WGPUSupportedLimits supportedLimits{};
    wgpuDeviceGetLimits(device, &supportedLimits);
    limits = supportedLimits.limits;

    depthTexture = WTexture::GetDepthTexture(device, width, height);
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
                depthTexture = WTexture::GetDepthTexture(device, config.width, config.height);
                wgpuSurfaceConfigure(surface, &config);
            }
            std::cout << "[WEngine]::[INFO]: Resizing in the main function!" << std::endl;
            skip = true;
        }
        case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
        case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
        case WGPUSurfaceGetCurrentTextureStatus_Force32:
            throw std::exception(fmt::format("[WEngine]::[ERROR]: get_current_texture status={}", (uint32_t)surfaceTexture.status).c_str());
    }

    if (!skip) {
        WGPUTextureView target = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
        frame(target);
        wgpuSurfacePresent(surface);

        wgpuTextureViewRelease(target);
        wgpuTextureRelease(surfaceTexture.texture);
    }
}

void WEngine::setupLogging(WGPULogLevel level) const {
    wgpuSetLogCallback(wgpuLogCallback, nullptr);
    wgpuSetLogLevel(level);
}

void WEngine::printWGPUReport() const {
    WGPUGlobalReport report{};
    wgpuGenerateReport(instance, &report);

    std::function<void(const char *, WGPURegistryReport)> printRegistryReport = [](const char *prefix, WGPURegistryReport report) {
        fmt::println("\nBegin {}", prefix);
        fmt::println("{}.numAllocated = {}", prefix, report.numAllocated);
        fmt::println("{}.numKeptFromUser = {}", prefix, report.numKeptFromUser);
        fmt::println("{}.numReleasedFromUser = {}", prefix, report.numReleasedFromUser);
        fmt::println("{}.numError = {}", prefix, report.numError);
        fmt::println("{}.elementSize = {}", prefix, report.elementSize);
        fmt::println("End {}\n", prefix);
    };
    std::function<void(const char *, WGPUHubReport)> printHubReport = [&](const char *prefix, WGPUHubReport report) {
        printRegistryReport(fmt::format("{}{}", prefix, "adapters").c_str(), report.adapters);
        printRegistryReport(fmt::format("{}{}", prefix, "devices").c_str(), report.devices);
        printRegistryReport(fmt::format("{}{}", prefix, "queues").c_str(), report.queues);
        printRegistryReport(fmt::format("{}{}", prefix, "pipelineLayouts").c_str(), report.pipelineLayouts);
        printRegistryReport(fmt::format("{}{}", prefix, "shaderModules").c_str(), report.shaderModules);
        printRegistryReport(fmt::format("{}{}", prefix, "bindGroupLayouts").c_str(), report.bindGroupLayouts);
        printRegistryReport(fmt::format("{}{}", prefix, "bindGroups").c_str(), report.bindGroups);
        printRegistryReport(fmt::format("{}{}", prefix, "commandBuffers").c_str(), report.commandBuffers);
        printRegistryReport(fmt::format("{}{}", prefix, "renderBundles").c_str(), report.renderBundles);
        printRegistryReport(fmt::format("{}{}", prefix, "renderPipelines").c_str(), report.renderPipelines);
        printRegistryReport(fmt::format("{}{}", prefix, "computePipelines").c_str(), report.computePipelines);
        printRegistryReport(fmt::format("{}{}", prefix, "querySets").c_str(), report.querySets);
        printRegistryReport(fmt::format("{}{}", prefix, "textures").c_str(), report.textures);
        printRegistryReport(fmt::format("{}{}", prefix, "textureViews").c_str(), report.textureViews);
        printRegistryReport(fmt::format("{}{}", prefix, "samplers").c_str(), report.samplers);
        printRegistryReport(fmt::format("{}{}", prefix, "buffers").c_str(), report.buffers);
    };

    fmt::println("------------------------[WEngine]::[INFO]::[WGPU_REPORT]------------------------------------");

    printRegistryReport("\tsurfaces", report.surfaces);

    switch (report.backendType) {
        case WGPUBackendType_D3D12:
            printHubReport("\tdx12.", report.dx12);
            break;
        case WGPUBackendType_Metal:
            printHubReport("\tmetal.", report.metal);
            break;
        case WGPUBackendType_Vulkan:
            printHubReport("\tvulkan.", report.vulkan);
            break;
        case WGPUBackendType_OpenGLES:
        case WGPUBackendType_OpenGL:
            printHubReport("\tgl.", report.gl);
            break;
        defualt:
            fmt::println("[WEngine]::[WARN]: There is no such backend type!");
    }

    fmt::println("--------------------------------------------------------------------------------------------");
}

void WEngine::glfwKeyCallback(GLFWwindow *window, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        WEngine *engine = (WEngine *)glfwGetWindowUserPointer(window);
        engine->printWGPUReport();
    }
}

void WEngine::glfwFramebuffersizeCallback(GLFWwindow *window, int32_t width, int32_t height) {
    if (width == 0 || height == 0) {
        return;
    }

    WEngine &engine = *(WEngine *)glfwGetWindowUserPointer(window);

    engine.width = width;
    engine.height = height;
    engine.config.width = width;
    engine.config.height = height;

    engine.depthTexture = WTexture::GetDepthTexture(engine.device, engine.width, engine.height);
    wgpuSurfaceConfigure(engine.surface, &engine.config);
}

void WEngine::wgpuLogCallback(WGPULogLevel level, const char *message, void *userdata) {
    const char *levelStr = "";
    switch (level) {
        case WGPULogLevel_Error:
            levelStr = "[WEngine]::[ERROR]::[WGPU]: ";
            break;
        case WGPULogLevel_Info:
            levelStr = "[WEngine]::[INFO]::[WGPU]: ";
            break;
        case WGPULogLevel_Debug:
            levelStr = "[WEngine]::[DEBUG]::[WGPU]: ";
            break;
        case WGPULogLevel_Warn:
            levelStr = "[WEngine]::[WARN]::[WGPU]: ";
            break;
        case WGPULogLevel_Trace:
            levelStr = "[WEngine]::[TRACE]::[WGPU]: ";
            break;
        default:
            levelStr = "[WEngine]::[UNKNOWN_LEVEL]: ";
    }
    fmt::println("{}{}", levelStr, message);
}

WGPUShaderModule WEngine::shaderFromWgslFile(WGPUDevice device, std::string path) {
    std::ifstream file{path};

    if (!file.is_open()) {
        throw std::exception(fmt::format("[WEngine]::[ERROR]: Failed to open shader file from path: {}", path).c_str());
    }
    std::stringstream shaderStream;
    shaderStream << file.rdbuf();
    file.close();

    std::string shaderCode = shaderStream.str();

    WGPUShaderModuleWGSLDescriptor wgslDescriptor{
        .chain = WGPUChainedStruct{
            .sType = WGPUSType_ShaderModuleWGSLDescriptor,
        },
        .code = shaderCode.c_str(),
    };
    WGPUShaderModuleDescriptor shaderDescriptor{
        .nextInChain = (const WGPUChainedStruct *)&wgslDescriptor,
        .label = path.c_str(),
    };

    return wgpuDeviceCreateShaderModule(device, &shaderDescriptor);
}
