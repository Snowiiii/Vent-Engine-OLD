#pragma once

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES

#include <imgui.h>
#include <backends/imgui_impl_sdl.h>

#include <backends/imgui_impl_vulkan.h>

#include <memory>

#include <Vent_Window.hpp>

class Vent_GUI
{
private:
    ImGuiContext *m_context;
    vk::DescriptorPool imgui_descriptor_pool;
    vk::RenderPass imgui_render_pass;

    void UpdateFontsTexture();

    void createImGuiDescriporPool();

    void createImGuiRenderPass();

public:
    Vent_GUI(const Vent_Window &window);
    ~Vent_GUI();

    void update(const vk::CommandBuffer &buffer);

    void handleSDLEvent(SDL_Event event);
};
