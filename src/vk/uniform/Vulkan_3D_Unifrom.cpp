#include "Vulkan_3D_Unifrom.hpp"

#define ZOOM -2.5

Vulkan_3D_Unifrom::Vulkan_3D_Unifrom(Camera &camera)
{
    this->uniform_buffer_general = std::make_unique<VulkanVertexBuffer>(context->device, sizeof(ubo_vs) * 1000, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);

    this->updateViewMatrix(camera);
}

Vulkan_3D_Unifrom::~Vulkan_3D_Unifrom()
{
    this->uniform_buffer_general.reset();
}

void Vulkan_3D_Unifrom::updateViewMatrix(Camera &camera)
{
    VkExtent2D extent = {context->swapchain_dimensions.width, context->swapchain_dimensions.height};
    ubo_vs.projection = glm::perspective(glm::radians(60.0f), (float)extent.width / (float)extent.height, 0.1f, 256.0f);
    ubo_vs.projection[1][1] *= -1;
    glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, ZOOM));

    ubo_vs.view = view_matrix * glm::translate(glm::mat4(1.0f), camera.pos);
    ubo_vs.view = glm::rotate(ubo_vs.view, glm::radians(camera.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo_vs.view = glm::rotate(ubo_vs.view, glm::radians(camera.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_vs.view = glm::rotate(ubo_vs.view, glm::radians(camera.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo_vs.view_pos = glm::vec4(0.0f, 0.0f, -ZOOM, 0.0f);

    uniform_buffer_general->convert_and_update(ubo_vs);
}

void Vulkan_3D_Unifrom::updateTransMatrix(const std::unique_ptr<Vulkan_Mesh> &object, const uint32_t &index)
{
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), object->pos);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), object->scale);

    ubo_vs.trans = glm::rotate(ubo_vs.trans, glm::radians(object->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo_vs.trans = glm::rotate(ubo_vs.trans, glm::radians(object->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_vs.trans = glm::rotate(ubo_vs.trans, glm::radians(object->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo_vs.view = ubo_vs.view * scaleMatrix * translationMatrix * ubo_vs.trans;

    uniform_buffer_general->convert_and_update(ubo_vs, index * sizeof(ubo_vs));
}