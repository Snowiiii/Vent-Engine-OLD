#include "ObjectRenderer.hpp"

ObjectRenderer::ObjectRenderer()
{
}

ObjectRenderer::~ObjectRenderer()
{
    for (auto &model : objects)
    {
        model.reset();
    }
    objects.clear();
}

void ObjectRenderer::addModel(std::unique_ptr<Vulkan_Mesh> &model)
{
    objects.push_back(std::move(model));
}

void ObjectRenderer::render(const vk::CommandBuffer &buffer, std::unique_ptr<Vulkan_3D_Unifrom> &uniform)
{
    for (size_t i = 0; i < objects.size(); i++)
    {
        const auto &model = objects[i];
        uniform->updateTransMatrix(model, i);
        model->bind(buffer, i);
        model->draw(buffer);
    }
}