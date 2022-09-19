#include "VulkanVertexBuffer.hpp"

VulkanVertexBuffer::VulkanVertexBuffer(
    vk::Device const &device,
    vk::DeviceSize size,
    VkBufferUsageFlags buffer_usage,
    VmaMemoryUsage memory_usage,
    VmaAllocationCreateFlags flags,
    const std::vector<uint32_t> &queue_family_indices) : size(size)
{

    persistent = (flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0;

    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.usage = buffer_usage;
    buffer_info.size = size;
    if (queue_family_indices.size() >= 2)
    {
        buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        buffer_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
        buffer_info.pQueueFamilyIndices = queue_family_indices.data();
    }

    VmaAllocationCreateInfo memory_info{};
    memory_info.flags = flags;
    memory_info.usage = memory_usage;

    VmaAllocationInfo allocation_info{};
    auto result = vmaCreateBuffer(context->memory_allocator,
                                  &buffer_info, &memory_info,
                                  &handle, &allocation,
                                  &allocation_info);
    
    if (result != VK_SUCCESS)
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Can't create VMA Buffer");
        return;
	}


    if (persistent)
    {
        mapped_data = static_cast<uint8_t *>(allocation_info.pMappedData);
    }
}

uint8_t *VulkanVertexBuffer::map()
{
    if (!mapped && !mapped_data)
    {
        vmaMapMemory(context->memory_allocator, allocation, reinterpret_cast<void **>(&mapped_data));
        mapped = true;
    }
    return mapped_data;
}

void VulkanVertexBuffer::unmap()
{
    if (mapped)
    {
        vmaUnmapMemory(context->memory_allocator, allocation);
        mapped_data = nullptr;
        mapped = false;
    }
}

void VulkanVertexBuffer::flush() const
{
    vmaFlushAllocation(context->memory_allocator, allocation, 0, size);
}

void VulkanVertexBuffer::update(void *data, size_t size, size_t offset)
{
    update(reinterpret_cast<const uint8_t *>(data), size, offset);
}

void VulkanVertexBuffer::update(const uint8_t *data, const size_t size, const size_t offset)
{
    if (persistent)
    {
        std::copy(data, data + size, mapped_data + offset);
        flush();
    }
    else
    {
        map();
        std::copy(data, data + size, mapped_data + offset);
        flush();
        unmap();
    }
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    if (handle != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
    {
        unmap(); 
        vmaDestroyBuffer(context->memory_allocator, handle, allocation);
    }
}
