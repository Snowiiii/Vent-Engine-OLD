#pragma once

#include "../Vulkan_Base.hpp"
#include "../Vulkan_Model.hpp"
#include "../buffer/VulkanVertexBuffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <memory>
#include <vector>

class Vulkan_Image
{
private:
    Texture texture;

    void createImage();
public:
    Vulkan_Image(Vulkan_Model model,const char* path);
    ~Vulkan_Image();
};


