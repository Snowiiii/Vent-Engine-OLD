#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#undef VMA_STATIC_VULKAN_FUNCTIONS 
#undef VMA_DYNAMIC_VULKAN_FUNCTIONS

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <vulkan/vulkan.hpp>

#include <spdlog/spdlog.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

class Vent_Window
{

public:
    SDL_Window *handle;

    uint32_t width, height;

    Vent_Window(uint32_t width, uint32_t height, const char *title);

    ~Vent_Window();

    Vent_Window(const Vent_Window &) = default;

    Vent_Window &operator=(const Vent_Window &) = delete;

    vk::Extent2D getExtent() const { return {width, height}; }

    vk::SurfaceKHR createSurface(vk::Instance instance);

    bool handleEvents();  

private:
};
