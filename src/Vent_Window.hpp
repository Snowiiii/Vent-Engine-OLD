#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#undef VMA_STATIC_VULKAN_FUNCTIONS 
#undef VMA_DYNAMIC_VULKAN_FUNCTIONS

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <vulkan/vulkan.hpp>

#include <spdlog/spdlog.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Vent_Window
{

public:
    uint32_t width, height;

    Vent_Window(uint32_t width, uint32_t height, const char *title);

    ~Vent_Window();

    Vent_Window(const Vent_Window &) = default;

    Vent_Window &operator=(const Vent_Window &) = delete;

    vk::SurfaceKHR createSurface(vk::Instance instance);

    vk::Extent2D getExtent() const { return {width, height}; }

    void update() const noexcept;

    bool shouldClose() const noexcept { return glfwWindowShouldClose(handle); }

private:
    GLFWwindow *handle{};

    void createWindow(const char* title);

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
};
