#pragma once

#include "../vk/Vulkan_Base.hpp"

#include "../vk/uniform/Vulkan_3D_Unifrom.hpp"
#include "../vk/Vulkan_Mesh.hpp"

#include <memory>
#include <vector>

class ObjectRenderer
{
private:
    std::vector<std::unique_ptr<Vulkan_Mesh>> objects;

public:
    ObjectRenderer();
    ~ObjectRenderer();

    void addModel(std::unique_ptr<Vulkan_Mesh> &model);

    void render(const vk::CommandBuffer &buffer, std::unique_ptr<Vulkan_3D_Unifrom> &uniform);
};
