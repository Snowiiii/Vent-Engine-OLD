#pragma once

#include "../Vulkan_Base.hpp"
#include "../Vulkan_Model.hpp"
#include "../buffer/VulkanVertexBuffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <memory>
#include <vector>

class Vulkan_Image
{
private:
    vk::CommandPool command_pool;
    vk::DescriptorSet descriptor_set;

    Texture texture;

    void createImage();

    void upload(stbi_uc* data, int width, int height);

    void createDescriptorSet();

    void flush_command_buffer(vk::CommandBuffer command_buffer, vk::Queue queue, bool free = true, vk::Semaphore signalSemaphore = VK_NULL_HANDLE) const;

public:
    Vulkan_Image(Vulkan_Model model, const char *path);
    ~Vulkan_Image();
};
