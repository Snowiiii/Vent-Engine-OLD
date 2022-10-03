#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#undef VMA_STATIC_VULKAN_FUNCTIONS
#undef VMA_DYNAMIC_VULKAN_FUNCTIONS

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <vulkan/vulkan.hpp>

#include <string_view>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include "objects/Camera.hpp"


struct ApplicationInfo {
    const std::string_view name;

    // Window
    int width = 1080;
    int height = 720;

    bool vsync = true;
};

class Vent_Window
{
public:
    SDL_Window *handle;

    uint32_t width, height;

    Vent_Window(const uint32_t &width, const uint32_t &height, const std::string_view &title);

    ~Vent_Window();

    Vent_Window(const Vent_Window &) = default;

    Vent_Window &operator=(const Vent_Window &) = delete;

    vk::SurfaceKHR createSurface(vk::Instance &instance);

    const char **getInstanceExtensions(uint32_t *count);

    bool handleEvents(SDL_Event &event, Camera &camera);

    bool isMouseGrabbed() { return _mouseGrabbed; }

    void grabMouse(bool grab);

private:
    bool _mouseGrabbed;
};
