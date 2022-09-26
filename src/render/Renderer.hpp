#pragma once

#include "../vk/Vulkan_Base.hpp"
#include "../vk/Vulkan_Mesh.hpp"

#include "ObjectRenderer.hpp"

#include "../vk/uniform/Vulkan_3D_Unifrom.hpp"

class Renderer
{
public:
    Vent_Window window{800, 800, "Vent-Engine Runtime"};
    Camera camera{};

    Renderer();

    ~Renderer();

    void update(float delta);

private:
    VKBase vkbase{window};

    std::unique_ptr<Vulkan_3D_Unifrom> uniform;

    std::unique_ptr<ObjectRenderer> objectRenderer;

    bool resize(const uint32_t,const uint32_t);

    void init_framebuffers();

    void teardown_framebuffers();

    void loadModels();

    void loadUniform();

    void render(uint32_t &swapchain_index);

    vk::Result acquire_next_image(uint32_t &image);

    vk::Result present_image(uint32_t &index);
};
