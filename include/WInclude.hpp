#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <memory>

#include <wgpu/wgpu.h>
#include <fmt/core.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

#include <glfw3wgpu.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define WENGINE_PLATFORM_WINDOWS
#include <stdexcept>
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define WENGINE_PLATFORM_LINUX
#if defined(__ANDROID__)
#define WENGINE_PLATFORM_ANDROID
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define WENGINE_PLATFORM_UNIX
#elif defined(_POSIX_VERSION)
// Posix
#define WENGINE_PLATFORM_POSIX
#elif __APPLE__
// Apple Platforms
#define WENGINE_PLATFORM_APPLE
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define WENGINE_PLATFORM_IOS
#define WENGINE_PLATFORM_IOS_SIMULATOR
#elif TARGET_OS_IPHONE
#define WENGINE_PLATFORM_IOS
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple Platform"
#endif
#else
#error "Unknown Platform!"
#endif

#include <WTypes.hpp>
