#pragma once

#include "../Vulkan_Base.hpp"
#include "../buffer/VulkanVertexBuffer.hpp"

#include <memory>
#include <vector>

class VulkanImage
{
private:
    vk::DescriptorSet descriptor_set;

    Texture texture;

    void createImage();

    void upload(u_char *data, uint32_t width, uint32_t height);

    void createDescriptorSet(const vk::DescriptorBufferInfo &buffer_descriptor);

    void flush_command_buffer(const vk::CommandBuffer &command_buffer, vk::Queue &queue, bool free, vk::Semaphore signalSemaphore = VK_NULL_HANDLE) const;

public:
    VulkanImage(const vk::DescriptorBufferInfo &buffer_descriptor, const std::string &path);

    // Vulkan_Image(const Vulkan_Image &) = delete;

    // Vulkan_Image &operator=(const Vulkan_Image &) = delete;

	// Vulkan_Image &operator=(Vulkan_Image &&) = delete;

    ~VulkanImage();

    void bind(const vk::CommandBuffer &buffer,const uint32_t index) const;
};
