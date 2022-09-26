#include "Vulkan_Mesh.hpp"

Vulkan_Mesh::Vulkan_Mesh(const std::string_view &path)
{
    OBJMeshFormatLoader loader(path);
    this->createBuffers(loader.getVertices(), loader.getIndices());
}

Vulkan_Mesh::Vulkan_Mesh(std::vector<Vertex> &pvertices, std::vector<uint32_t> &pindices)
{
    this->createBuffers(pvertices, pindices);
}

Vulkan_Mesh::~Vulkan_Mesh()
{
    if (image)
    {
        image.reset();
    }

    vertex_buffer.reset();
    index_buffer.reset();
}

void Vulkan_Mesh::createBuffers(std::vector<Vertex> &pvertices, std::vector<uint32_t> &pindices)
{
    // vertex

    vertexCount = static_cast<uint32_t>(pvertices.size() * sizeof(Vertex));

    vertex_buffer = std::make_unique<VulkanVertexBuffer>(context->device, vertexCount, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);
    vertex_buffer->update(pvertices.data(), vertexCount);

    // index

    index_count = static_cast<uint32_t>(pindices.size());

    const auto index_buffer_size = static_cast<uint32_t>((pindices.size() * sizeof(uint32_t)));

    index_buffer = std::make_unique<VulkanVertexBuffer>(context->device, index_buffer_size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);
    index_buffer->update(pindices.data(), index_buffer_size);
}

uint32_t Vulkan_Mesh::get_memory_type(uint32_t bits, vk::MemoryPropertyFlags properties, vk::Bool32 *memory_type_found)
{
    const vk::PhysicalDeviceMemoryProperties memory_properties = context->gpu.getMemoryProperties();

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if ((bits & 1) == 1)
        {
            if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memory_type_found)
                {
                    *memory_type_found = true;
                }
                return i;
            }
        }
        bits >>= 1;
    }

    if (memory_type_found)
    {
        *memory_type_found = false;
        return ~0;
    }
    else
    {
        throw std::runtime_error("Could not find a matching memory type");
    }
}

void Vulkan_Mesh::bind(const vk::CommandBuffer &commandBuffer, const uint32_t index) const
{
    if (image)
    {
        image->bind(commandBuffer, index);
    }

    vk::DeviceSize offset = 0;
    commandBuffer.bindVertexBuffers2(0, vertex_buffer->get_handle(), offset);
    commandBuffer.bindIndexBuffer(index_buffer->get_handle(), 0, vk::IndexType::eUint32);
}

void Vulkan_Mesh::setTexture(const vk::DescriptorBufferInfo &buffer_descriptor, const std::string &path)
{
    image = std::make_unique<VulkanImage>(buffer_descriptor, path);
}

void Vulkan_Mesh::draw(const vk::CommandBuffer &commandBuffer) const
{
    commandBuffer.drawIndexed(index_count, 1, 0, 0, 0);
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
{
    const std::array<vk::VertexInputAttributeDescription, 3> vertex_input_attributes = {
        {{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)},      // Location 0 : Position
         {1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},          // Location 1: Texture Coordinates
         {2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)}}}; // Location 2 : Normal
    return vertex_input_attributes;
}

vk::VertexInputBindingDescription Vertex::getBindingDescription()
{
    const vk::VertexInputBindingDescription vertex_input_binding(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
    return vertex_input_binding;
}