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

    void createSampleAndView(const vk::Format &format, const bool sample);

    void createImage(const vk::Format &format, const vk::ImageUsageFlags imageflags);

    void createAndUpload(u_char *data, uint32_t width, uint32_t height, const vk::Format &format);

    void createDescriptorSet(const vk::DescriptorBufferInfo &buffer_descriptor);

    void flush_command_buffer(const vk::CommandBuffer &command_buffer, vk::Queue &queue, bool free, vk::Semaphore signalSemaphore = VK_NULL_HANDLE) const;

public:
    VulkanImage(const vk::Format &format, const uint32_t &width, const uint32_t &height);

    VulkanImage(const vk::DescriptorBufferInfo &buffer_descriptor, const std::string_view &path);

    ~VulkanImage();

    void bind(const vk::CommandBuffer &buffer, const uint32_t &index) const;

    const Texture &getTexture() const { return texture; };
};
