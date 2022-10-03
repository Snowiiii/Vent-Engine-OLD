#pragma once

#include "../vk/Vulkan_Base.hpp"
#include "../vk/mesh/Vulkan_Mesh.hpp"

#include "ObjectRenderer.hpp"

#include "../vk/uniform/Vulkan_3D_Unifrom.hpp"

class Renderer
{
public:
    Vent_Window window{800, 800, "Vent-Engine Runtime"};
    Camera camera{};

    Renderer();

    ~Renderer();

    uint32_t onPreUpdate(float delta);

    void onPostUpdate(uint32_t &index);

    vk::CommandBuffer onPreDraw(uint32_t &swapchain_index);

    void onPostDraw(const vk::CommandBuffer &cmd,uint32_t &swapchain_index);

private:
    VKBase vkbase{window};

    std::unique_ptr<Vulkan_3D_Unifrom> uniform;

    std::unique_ptr<ObjectRenderer> objectRenderer;

    std::unique_ptr<VulkanImage> depth_image;


    bool resize(const uint32_t,const uint32_t);

    void init_framebuffers();

    void teardown_framebuffers();

    void loadModels();

    void loadUniform();

    vk::Result acquire_next_image(uint32_t &image);

    vk::Result present_image(uint32_t &index);
};
