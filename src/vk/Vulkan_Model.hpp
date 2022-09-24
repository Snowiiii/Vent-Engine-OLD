#pragma once

#include "Vulkan_Base.hpp"
#include "buffer/VulkanVertexBuffer.hpp"

#include <memory>
#include <vector>

class Vulkan_Model
{
private:
    std::unique_ptr<VulkanVertexBuffer> vertex_buffer;
    std::unique_ptr<VulkanVertexBuffer> index_buffer;

    uint32_t vertexCount;
    uint32_t index_count;

public:
    Vulkan_Model(std::vector<Vertex> &vertices, std::vector<uint32_t> &pindices);
    ~Vulkan_Model();

    Vulkan_Model(const Vulkan_Model &) = delete;
    Vulkan_Model &operator=(const Vulkan_Model &) = delete;

    static uint32_t get_memory_type(uint32_t bits, vk::MemoryPropertyFlags properties, vk::Bool32 *memory_type_found = nullptr);

    void bind(const vk::CommandBuffer &commandBuffer) const;
    void draw(const vk::CommandBuffer &commandBuffer) const;
};
