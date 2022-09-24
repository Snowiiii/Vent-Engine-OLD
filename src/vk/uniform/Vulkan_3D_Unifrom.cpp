#include "Vulkan_3D_Unifrom.hpp"

#define ZOOM -2.5

Vulkan_3D_Unifrom::Vulkan_3D_Unifrom(Camera &camera)
{
    this->uniform_buffer_vs = std::make_unique<VulkanVertexBuffer>(context->device, sizeof(ubo_vs), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);

    this->updateUniformBuffers(camera);
}

Vulkan_3D_Unifrom::~Vulkan_3D_Unifrom() {
    this->uniform_buffer_vs.reset();
}

void Vulkan_3D_Unifrom::updateUniformBuffers(Camera &camera)
{
    VkExtent2D extent = {context->swapchain_dimensions.width, context->swapchain_dimensions.height};
    ubo_vs.projection = glm::perspective(glm::radians(60.0f), (float)extent.width / (float)extent.height, 0.001f, 256.0f);
    glm::mat4 view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, ZOOM));

    ubo_vs.model = view_matrix * glm::translate(glm::mat4(1.0f), camera.camera_pos);
    ubo_vs.model = glm::rotate(ubo_vs.model, glm::radians(camera.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo_vs.model = glm::rotate(ubo_vs.model, glm::radians(camera.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_vs.model = glm::rotate(ubo_vs.model, glm::radians(camera.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo_vs.view_pos = glm::vec4(0.0f, 0.0f, -ZOOM, 0.0f);

    uniform_buffer_vs->convert_and_update(ubo_vs);
}