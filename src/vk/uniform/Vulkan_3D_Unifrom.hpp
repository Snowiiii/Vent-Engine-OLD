#pragma once
#include "../Vulkan_Base.hpp"
#include "../buffer/VulkanVertexBuffer.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "../Vulkan_Mesh.hpp"

#include <memory>

class Vulkan_3D_Unifrom
{
private:
public:
    struct
    {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 trans;
        glm::vec4 view_pos;
        float lod_bias = 0.0f;
    } ubo_vs;
    std::unique_ptr<VulkanVertexBuffer> uniform_buffer_general;

    explicit Vulkan_3D_Unifrom(Camera &camera);
    ~Vulkan_3D_Unifrom();

    Vulkan_3D_Unifrom(const Vulkan_3D_Unifrom &) = default;

    Vulkan_3D_Unifrom &operator=(const Vulkan_3D_Unifrom &) = delete;

    void updateViewMatrix(Camera &camera);

    void updateTransMatrix(const std::unique_ptr<Vulkan_Mesh> &object, const uint32_t &index);

    vk::Buffer getBuffer() const { return uniform_buffer_general->get_handle(); }
};
