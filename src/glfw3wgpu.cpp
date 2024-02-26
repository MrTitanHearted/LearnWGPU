#include <glfw3wgpu.hpp>

#define WGPU_TARGET_MACOS 1
#define WGPU_TARGET_LINUX_X11 2
#define WGPU_TARGET_WINDOWS 3
#define WGPU_TARGET_LINUX_WAYLAND 4

#if defined(_WIN32)
#define WGPU_TARGET WGPU_TARGET_WINDOWS
#elif defined(__APPLE__)
#define WGPU_TARGET WGPU_TARGET_MACOS
#else
#define WGPU_TARGET WGPU_TARGET_LINUX_X11
#endif

#if WGPU_TARGET == WGPU_TARGET_MACOS
#include <Foundation/Foundation.h>
#include <QuartzCore/CAMetalLayer.h>
#endif

#if WGPU_TARGET == WGPU_TARGET_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif WGPU_TARGET == WGPU_TARGET_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

WGPUSurface glfwGetWGPUSurface(GLFWwindow* window, WGPUInstance instance) {
#if WGPU_TARGET == WGPU_TARGET_MACOS
    {
        id metal_layer = NULL;
        NSWindow* ns_window = glfwGetCocoaWindow(window);
        [ns_window.contentView setWantsLayer:YES];
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer:metal_layer];
        WGPUSurfaceDescriptorFromMetalLayer surfaceFromMetalLayer{
            .chain = WGPUChainedStruct{
                .next = NULL,
                .sType = WGPUSType_SurfaceDescriptorFromMetalLayer,
            },
            .layer = metal_layer,
        };
        WGPUSurfaceDescriptor surfaceDescriptor{
            .nextInChain = (const WGPUChainedStruct*)&surfaceFromMetalLayer,
            .label = "WebGPU Native Metal GLFW3 Surface",
        };
        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif WGPU_TARGET == WGPU_TARGET_LINUX_X11
    {
        Display* x11_display = glfwGetX11Display();
        Window x11_window = glfwGetX11Window(window);
        WGPUSurfaceDescriptorFromXlibWindow surfaceFromXlibWindow{
            .chain = WGPUChainedStruct{
                .next = NULL,
                .sType = WGPUSType_SurfaceDescriptorFromXlibWindow,
            },
            .display = x11_display,
            .window = x11_window,
        };
        WGPUSurfaceDescriptor surfaceDescriptor{
            .nextInChain = (const WGPUChainedStruct*)&surfaceFromXlibWindow,
            .label = "WebGPU Native Linux GLFW3 Surface",
        };
        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif WGPU_TARGET == WGPU_TARGET_LINUX_WAYLAND
    {
        struct wl_display* wayland_display = glfwGetWaylandDisplay();
        struct wl_surface* wayland_surface = glfwGetWaylandWindow(window);
        WGPUSurfaceDescriptorFromWaylandSurface surfaceFromWaylandWindow{
            .chain = WGPUChainedStruct{
                .next = NULL,
                .sType = WGPUSType_SurfaceDescriptorFromWaylandSurface,
            },
            .display = wayland_display,
            .surface = wayland_surface,
        };
        WGPUSurfaceDescriptor surfaceDescriptor{
            .nextInChain = (const WGPUChainedStruct*)&surfaceFromWaylandWindow,
            .label = "WebGPU Native Linux Wayland GLFW3 Surface",
        };
        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif WGPU_TARGET == WGPU_TARGET_WINDOWS
    {
        HWND hwnd = glfwGetWin32Window(window);
        HINSTANCE hinstance = GetModuleHandle(NULL);
        WGPUSurfaceDescriptorFromWindowsHWND surfaceFromHWNDWindow{
            .chain WGPUChainedStruct{
                .next = NULL,
                .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND,
            },
            .hinstance = hinstance,
            .hwnd = hwnd,
        };
        WGPUSurfaceDescriptor surfaceDescriptor{
            .nextInChain =
                (const WGPUChainedStruct*)&surfaceFromHWNDWindow,
            .label = "WebGPU Native Windows GLFW3 Surface",
        };
        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#else
#error "Unsupported WGPU_TARGET"
#endif
}
