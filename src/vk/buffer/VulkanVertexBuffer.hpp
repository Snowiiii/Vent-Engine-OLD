#pragma once

#include "../Vulkan_Base.hpp"

class VulkanVertexBuffer
{
private:
	VkBuffer handle;
	VmaAllocation allocation{VK_NULL_HANDLE};
	VkDeviceSize size{0};
	uint8_t *mapped_data{nullptr};

	bool persistent{false};

	/// Whether the buffer has been mapped with vmaMapMemory
	bool mapped{false};

public:
	VulkanVertexBuffer(vk::Device const &device,
					   vk::DeviceSize size,
					   vk::BufferUsageFlags buffer_usage,
					   VmaMemoryUsage memory_usage,
					   VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT) : VulkanVertexBuffer(reinterpret_cast<vk::Device const &>(device),
																											   static_cast<VkDeviceSize>(size),
																											   static_cast<VkBufferUsageFlags>(buffer_usage),
																											   memory_usage,
																											   flags) {}

	VulkanVertexBuffer(vk::Device const &device,
					   vk::DeviceSize size,
					   VkBufferUsageFlags buffer_usage,
					   VmaMemoryUsage memory_usage,
					   VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
					   const std::vector<uint32_t> &queue_family_indices = {});
	~VulkanVertexBuffer();

	VulkanVertexBuffer &operator=(const VulkanVertexBuffer &) = delete;

	VulkanVertexBuffer &operator=(VulkanVertexBuffer &&) = delete;

	void flush() const;

	uint8_t *map();

	void unmap();

	void update(const uint8_t *data, size_t size, size_t offset = 0);

	void update(void *data, size_t size, size_t offset = 0);

	void update(const std::vector<uint8_t> &data, size_t offset = 0);

	template <class T>
	void convert_and_update(const T &object, size_t offset = 0)
	{
		update(reinterpret_cast<const uint8_t *>(&object), sizeof(T), offset);
	}

	vk::Buffer get_handle() const
	{
		return static_cast<vk::Buffer>(handle);
	}

	vk::DeviceSize get_size() const
	{
		return static_cast<vk::DeviceSize>(size);
	}
};
