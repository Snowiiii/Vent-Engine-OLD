#include "Vulkan_Image.hpp"

Vulkan_Image::Vulkan_Image(Vulkan_Model model, const char *path)
{

    int width, height, channels;
    auto *data = stbi_load("../data/images/logo.png", &width, &height, &channels, 4);
    if (!data)
    {
        spdlog::warn("Could not load image data");
        return;
    }
}

void Vulkan_Image::createImage()
{
    vk::ImageCreateInfo image_create_info;
    image_create_info.imageType = vk::ImageType::e2D;
    image_create_info.format = vk::Format::eR8G8B8A8Srgb;
    image_create_info.mipLevels = texture.mip_levels;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = vk::SampleCountFlagBits::e1;
    image_create_info.tiling = vk::ImageTiling::eOptimal;
    image_create_info.sharingMode = vk::SharingMode::eExclusive;
    // Set initial layout of the image to undefined
    image_create_info.initialLayout = vk::ImageLayout::eUndefined;
    image_create_info.extent = vk::Extent3D(texture.extent, 1);
    image_create_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    texture.image = context->device.createImage(image_create_info);

    vk::MemoryRequirements memory_requirements = context->device.getImageMemoryRequirements(texture.image);
    vk::MemoryAllocateInfo memory_allocate_info = {memory_requirements.size,
                                                   Vulkan_Model::get_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
    texture.device_memory = context->device.allocateMemory(memory_allocate_info);
    context->device.bindImageMemory(texture.image, texture.device_memory, 0);

    vk::ImageViewCreateInfo view;
	view.viewType   = vk::ImageViewType::e2D;
	view.format     = vk::Format::eR8G8B8A8Srgb;
	view.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
	// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
	// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
	view.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
	view.subresourceRange.baseMipLevel   = 0;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount     = 1;
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = texture.mip_levels;
	// The view will be based on the texture's image
	view.image   = texture.image;
	texture.view = context->device.createImageView(view);
}