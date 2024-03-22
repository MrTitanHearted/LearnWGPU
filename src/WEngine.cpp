#include <WEngine.hpp>

#include <WUtils.hpp>
#include <WModel.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

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
    WGPUShaderModule modelShader = shaderFromWgslFile(device, "assets/shaders/model.wgsl");

    WGPUSampler sampler = WSamplerBuilder::New().build(device);

    struct Camera {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
    };

    Camera cameraData{
        .projection = camera.getProjectionMatrix((float)width / (float)height),
        .view = camera.getViewMatrix(),
    };
    WUniformBuffer cameraBuffer = WUniformBuffer::New(device, &cameraData, sizeof(Camera));
    WBindGroup globalGroup =
        WBindGroupBuilder::New()
            .addBindingSampler(0, sampler)
            .addBindingUniform(1, cameraBuffer)
            .build(device);

    WModel model =
        WModelBuilder::New()
            .setPath("assets/models/vanguard/punching.dae")
            .setColorTarget(config.format)
            .setGlobalBindGroup(globalGroup)
            .setVertexShader(modelShader)
            .setFragmentShader(modelShader)
            .buildFromFile(device);

    modelData = glm::scale(modelData, glm::vec3(scale));
    model.updateModel(queue, modelData);

    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float currentFrame = glfwGetTime();
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;

        modelData = glm::scale(glm::mat4{1.0f}, glm::vec3(scale));
        model.updateModel(queue, modelData);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processCameraMovement(WCameraMovement::WORLD_FORWARD, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processCameraMovement(WCameraMovement::WORLD_BACKWARD, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processCameraMovement(WCameraMovement::RIGHT, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processCameraMovement(WCameraMovement::LEFT, dt);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.processCameraMovement(WCameraMovement::WORLD_DOWN, dt);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.processCameraMovement(WCameraMovement::WORLD_UP, dt);

        cameraData.projection = camera.getProjectionMatrix((float)width / (float)height);
        cameraData.view = camera.getViewMatrix();
        cameraBuffer.update(queue, &cameraData);

        presentFrame([&](WGPUTextureView frame) {
            WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(device, nullptr);
            std::vector<WGPUCommandBuffer> commandBuffers{};

            WGPURenderPassEncoder encoder =
                WRenderPassBuilder::New()
                    .addColorTarget(WColorAttachment::New(frame).setClearColor(0.2, 0.3, 0.3, 1.0))
                    .setDepthAttachment(WDepthStencilAttachment::New(depthTexture))
                    .build(commandEncoder);
            model.render(encoder);

            updateImGui(encoder);
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
    // setupLogging();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetFramebufferSizeCallback(window, glfwFramebuffersizeCallback);
    glfwSetCursorPosCallback(window, glfwCursorPosCallback);
    glfwSetScrollCallback(window, glfwScrollCallabck);

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

    initImGui();
}

WEngine::~WEngine() {
    shutdownImGui();

    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuSurfaceRelease(surface);
    wgpuInstanceRelease(instance);

    glfwDestroyWindow(window);
    window = nullptr;
}

void WEngine::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO();

    ImGui_ImplWGPU_InitInfo info{};
    info.Device = device;
    info.RenderTargetFormat = config.format;
    info.DepthStencilFormat = WGPUTextureFormat_Depth32Float;

    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplWGPU_Init(&info);
}

void WEngine::shutdownImGui() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
}

void WEngine::updateImGui(WGPURenderPassEncoder encoder) {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Scale the model")) {
        ImGui::SliderFloat("Scale", &scale, 1.0f / 50.0f, 1.0f);
        ImGui::Text("FPS: %d, ms: %f", (uint32_t)(1.0f/dt), dt);

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), encoder);
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

void WEngine::glfwCursorPosCallback(GLFWwindow *window, double x, double y) {
    WEngine *engine = (WEngine *)glfwGetWindowUserPointer(window);
    engine->camera.processMouseMovement(x, y);
}

void WEngine::glfwScrollCallabck(GLFWwindow *window, double x, double y) {
    WEngine *engine = (WEngine *)glfwGetWindowUserPointer(window);
    engine->camera.processMouseScroll(y);
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
