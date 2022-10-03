#pragma once

#include "../Vulkan_Base.hpp"
#include "../buffer/VulkanVertexBuffer.hpp"

#include "../image/Vulkan_Image.hpp"

#include "format/MeshFormatLoader.hpp"

#include <memory>
#include <vector>
#include <string>

class Vulkan_Mesh : public GameObject
{
private:
    MeshFormatLoader loader;

    std::unique_ptr<VulkanVertexBuffer> vertex_buffer;
    std::unique_ptr<VulkanVertexBuffer> index_buffer;
    std::unique_ptr<VulkanImage> image;

    uint32_t vertexCount;
    uint32_t index_count;

    void createBuffers(std::vector<Vertex> &pvertices, std::vector<uint32_t> &pindices);

public:
    Vulkan_Mesh(const std::string &path);
    Vulkan_Mesh(std::vector<Vertex> &pvertices, std::vector<uint32_t> &pindices);
    ~Vulkan_Mesh();

    Vulkan_Mesh(const Vulkan_Mesh &) = delete;
    Vulkan_Mesh &operator=(const Vulkan_Mesh &) = delete;

    static uint32_t get_memory_type(uint32_t bits, vk::MemoryPropertyFlags properties, vk::Bool32 *memory_type_found = nullptr);

    void setTexture(const vk::DescriptorBufferInfo &buffer_descriptor, const std::string &path);

    void bind(const vk::CommandBuffer &commandBuffer, const uint32_t &index) const;
    void draw(const vk::CommandBuffer &commandBuffer) const;
};
