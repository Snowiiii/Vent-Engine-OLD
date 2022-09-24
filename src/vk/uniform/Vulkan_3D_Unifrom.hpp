#pragma once
#include "../Vulkan_Base.hpp"
#include "../buffer/VulkanVertexBuffer.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

class Vulkan_3D_Unifrom 
{
private:
    struct
    {
        glm::mat4 projection;
        glm::mat4 model;
        glm::vec4 view_pos;
        float lod_bias = 0.0f;
    } ubo_vs;

    std::unique_ptr<VulkanVertexBuffer> uniform_buffer_vs;

public:
    Vulkan_3D_Unifrom(Camera &camera);
    ~Vulkan_3D_Unifrom();

    void updateUniformBuffers(Camera &camera);

    vk::Buffer getBuffer() { return uniform_buffer_vs->get_handle(); }
};
