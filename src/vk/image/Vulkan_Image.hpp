#pragma once

#include "../Vulkan_Base.hpp"
#include "../Vulkan_Model.hpp"
#include "../buffer/VulkanVertexBuffer.hpp"

#include <memory>
#include <vector>

class Vulkan_Image
{
private:
    // vk::CommandPool command_pool;
    vk::DescriptorSet descriptor_set;

    Texture texture;

    void createImage();

    void upload(u_char *data, uint32_t width, uint32_t height);

    void createDescriptorSet(const vk::DescriptorBufferInfo &buffer_descriptor);

    void flush_command_buffer(const vk::CommandBuffer &command_buffer, vk::Queue queue, bool free = true, vk::Semaphore signalSemaphore = VK_NULL_HANDLE) const;

public:
    Vulkan_Image(const vk::DescriptorBufferInfo &buffer_descriptor, const std::string &path);
    ~Vulkan_Image();

    void bind(const vk::CommandBuffer &buffer) const;
};
