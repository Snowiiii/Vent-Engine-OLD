#include "Vulkan_3D_Unifrom.hpp"

#define ZOOM -2.5

[[nodiscard]] inline glm::mat4 reverse_depth_projection_matrix_lh(const float field_of_view, const float aspect_ratio, const float near_plane, const float far_plane) noexcept
{
    const auto tan_half_fov_y = glm::tan(glm::radians(field_of_view) / 2.0f);
    const auto far_minus_near = far_plane - near_plane;

    glm::mat4 result(0.0f);
    result[0][0] = 1.0f / (aspect_ratio * tan_half_fov_y);
    result[1][1] = -1.0f / (tan_half_fov_y);
    result[2][2] = far_plane / far_minus_near - 1.0f;
    result[2][3] = -1.0f;
    result[3][2] = (far_plane * near_plane) / far_minus_near;

    return result;
}

[[nodiscard]] inline glm::vec3 direction_from_rotation(const glm::vec3 &rotation) noexcept
{
    const auto cos_y = glm::cos(rotation.y);

    glm::vec3 v;
    v.x = glm::sin(rotation.x) * cos_y;
    v.y = glm::sin(rotation.y);
    v.z = glm::cos(rotation.x) * cos_y;
    return v;
}

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
    vk::Extent2D extent = {context->swapchain_dimensions.width, context->swapchain_dimensions.height};
    ubo_vs.projection = reverse_depth_projection_matrix_lh(60.0f, (float)extent.width / (float)extent.height, 0.1f, 10000.0f);
    const auto look_at = camera.position + direction_from_rotation(camera.rotation);

    glm::mat4 view_matrix = glm::lookAt(camera.position, look_at, glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_vs.view = view_matrix;
    ubo_vs.view_pos = glm::vec4(0.0f, 0.0f, -ZOOM, 0.0f);

    uniform_buffer_general->convert_and_update(ubo_vs);
}

void Vulkan_3D_Unifrom::updateTransMatrix(const std::unique_ptr<Vulkan_Mesh> &object, const uint32_t &index)
{
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), object->position);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), object->scale);

    ubo_vs.trans = glm::rotate(ubo_vs.trans, glm::radians(object->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    ubo_vs.trans = glm::rotate(ubo_vs.trans, glm::radians(object->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo_vs.trans = glm::rotate(ubo_vs.trans, glm::radians(object->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo_vs.view = ubo_vs.view * scaleMatrix * translationMatrix * ubo_vs.trans;

    uniform_buffer_general->convert_and_update(ubo_vs, index * sizeof(ubo_vs));
}