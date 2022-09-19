#include "Vulkan_Image.hpp"

Vulkan_Image::Vulkan_Image(Vulkan_Model model, const char *path)
{

    int width, height, channels;
    auto *data = stbi_load("../data/images/logo.png", &width, &height, &channels, 4);
    if (!data)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not load image data");
        return;
    }

    texture.extent.width = width;
    texture.extent.height = height;
    texture.mip_levels = channels;

    this->createImage();
    this->upload(data, width, height);
    stbi_image_free(data);
    this->createDescriptorSet();
}

Vulkan_Image::~Vulkan_Image()
{
    context->device.destroyImageView(texture.view);
    context->device.destroyImage(texture.image);
    context->device.destroySampler(texture.sampler);
    context->device.freeMemory(texture.device_memory);
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

    vk::SamplerCreateInfo sampler;
    sampler.magFilter = vk::Filter::eLinear;
    sampler.minFilter = vk::Filter::eLinear;
    sampler.mipmapMode = vk::SamplerMipmapMode::eLinear;
    sampler.addressModeU = vk::SamplerAddressMode::eRepeat;
    sampler.addressModeV = vk::SamplerAddressMode::eRepeat;
    sampler.addressModeW = vk::SamplerAddressMode::eRepeat;
    sampler.mipLodBias = 0.0f;
    sampler.compareOp = vk::CompareOp::eNever;
    sampler.minLod = 0.0f;
    // Set max level-of-detail to mip level count of the texture
    sampler.maxLod = texture.mip_levels;
    sampler.maxAnisotropy = 1.0f;
    // Enable anisotropic filtering
    // This feature is optional, so we must check if it's supported on the device
    if (context->gpu.getFeatures().samplerAnisotropy)
    {
        // Use max. level of anisotropy for this example
        sampler.maxAnisotropy = context->gpu.getProperties().limits.maxSamplerAnisotropy;
        sampler.anisotropyEnable = true;
    }
    else
    {
        // The device does not support anisotropic filtering
        sampler.maxAnisotropy = 1.0;
        sampler.anisotropyEnable = false;
    }
    sampler.borderColor = vk::BorderColor::eFloatOpaqueWhite;
    texture.sampler = context->device.createSampler(sampler);

    vk::ImageViewCreateInfo view;
    view.viewType = vk::ImageViewType::e2D;
    view.format = vk::Format::eR8G8B8A8Srgb;
    view.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
    // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
    // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
    view.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    // Linear tiling usually won't support mip maps
    // Only set mip map count if optimal tiling is used
    view.subresourceRange.levelCount = texture.mip_levels;
    // The view will be based on the texture's image
    view.image = texture.image;
    texture.view = context->device.createImageView(view);
}

void Vulkan_Image::upload(stbi_uc *pdata, int width, int height)
{
    vk::BufferCreateInfo buffer_create_info({}, sizeof(pdata), vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, {});
    vk::Buffer staging_buffer = context->device.createBuffer(buffer_create_info);

    // Get memory requirements for the staging buffer (alignment, memory type bits)
    vk::MemoryRequirements memory_requirements = context->device.getBufferMemoryRequirements(staging_buffer);
    vk::MemoryAllocateInfo memory_allocate_info(
        memory_requirements.size,
        // Get memory type index for a host visible buffer
        Vulkan_Model::get_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
    vk::DeviceMemory staging_memory = context->device.allocateMemory(memory_allocate_info);
    context->device.bindBufferMemory(staging_buffer, staging_memory, 0);

    // Copy texture data into host local staging buffer

    uint8_t *data = reinterpret_cast<uint8_t *>(context->device.mapMemory(staging_memory, 0, memory_requirements.size));
    memcpy(data, pdata, sizeof(pdata));
    context->device.unmapMemory(staging_memory);

    // Setup buffer copy regions for each mip level
    std::vector<vk::BufferImageCopy> buffer_copy_regions(texture.mip_levels);
    for (uint32_t i = 0; i < texture.mip_levels; i++)
    {
        buffer_copy_regions[i].imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        buffer_copy_regions[i].imageSubresource.mipLevel = i;
        buffer_copy_regions[i].imageSubresource.baseArrayLayer = 0;
        buffer_copy_regions[i].imageSubresource.layerCount = 1;
        buffer_copy_regions[i].imageExtent.width = width >> i;
        buffer_copy_regions[i].imageExtent.height = height >> i;
        buffer_copy_regions[i].imageExtent.depth = 1;
        // buffer_copy_regions[i].bufferOffset = offset;
    }

    // Create optimal tiled target image on the device
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

    memory_requirements = context->device.getImageMemoryRequirements(texture.image);
    memory_allocate_info = {memory_requirements.size,
                            Vulkan_Model::get_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)};
    texture.device_memory = context->device.allocateMemory(memory_allocate_info);
    context->device.bindImageMemory(texture.image, texture.device_memory, 0);

    vk::CommandBuffer copy_command = context->device.allocateCommandBuffers({command_pool, vk::CommandBufferLevel::ePrimary, 1}).front();
    copy_command.begin(vk::CommandBufferBeginInfo());

    // Image memory barriers for the texture image

    // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
    vk::ImageSubresourceRange subresource_range;
    // Image only contains color data
    subresource_range.aspectMask = vk::ImageAspectFlagBits::eColor;
    // Start at first mip level
    subresource_range.baseMipLevel = 0;
    // We will transition on all mip levels
    subresource_range.levelCount = texture.mip_levels;
    // The 2D texture only has one layer
    subresource_range.layerCount = 1;

    // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
    vk::ImageMemoryBarrier image_memory_barrier;
    image_memory_barrier.image = texture.image;
    image_memory_barrier.subresourceRange = subresource_range;
    image_memory_barrier.srcAccessMask = {};
    image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    image_memory_barrier.oldLayout = vk::ImageLayout::eUndefined;
    image_memory_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
    // Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
    copy_command.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, image_memory_barrier);

    // Copy mip levels from staging buffer
    copy_command.copyBufferToImage(staging_buffer, texture.image, vk::ImageLayout::eTransferDstOptimal, buffer_copy_regions);

    // Once the data has been uploaded we transfer the texture image to the shader read layout, so it can be sampled from
    image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    image_memory_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    image_memory_barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
    // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
    copy_command.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, image_memory_barrier);

    // Store current layout for later reuse
    texture.image_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

    flush_command_buffer(copy_command, context->queue, true);

    // Clean up staging resources
    context->device.freeMemory(staging_memory);
    context->device.destroyBuffer(staging_buffer);
}

void Vulkan_Image::createDescriptorSet()
{
    vk::DescriptorSetAllocateInfo alloc_info(context->descriptor_pool, 1, &context->descriptor_set_layout);

    descriptor_set = context->device.allocateDescriptorSets(alloc_info).front();

    vk::DescriptorImageInfo image_descriptor;
    image_descriptor.imageView = texture.view;
    image_descriptor.sampler = texture.sampler;
    image_descriptor.imageLayout = texture.image_layout;

    std::array<vk::WriteDescriptorSet, 2> write_descriptor_sets = {
        {// Binding 0 : Vertex shader uniform buffer
         {descriptor_set, 0, {}, vk::DescriptorType::eUniformBuffer, {}},
         // Binding 1 : Fragment shader texture sampler
         //	Fragment shader: layout (binding = 1) uniform sampler2D samplerColor;
         {descriptor_set, 1, {}, vk::DescriptorType::eCombinedImageSampler, image_descriptor}}};

    context->device.updateDescriptorSets(write_descriptor_sets, {});
}

void Vulkan_Image::flush_command_buffer(vk::CommandBuffer command_buffer, vk::Queue queue, bool free, vk::Semaphore signalSemaphore) const
{
    if (!command_buffer)
    {
        return;
    }

    command_buffer.end();

    vk::SubmitInfo submit_info({}, {}, command_buffer);
    if (signalSemaphore)
    {
        submit_info.setSignalSemaphores(signalSemaphore);
    }

    // Create fence to ensure that the command buffer has finished executing
    vk::Fence fence = context->device.createFence({});

    // Submit to the queue
    queue.submit(submit_info, fence);

    // Wait for the fence to signal that command buffer has finished executing
    vk::Result result = context->device.waitForFences(fence, true, 100000000000);
    if (result != vk::Result::eSuccess)
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to flush Command Buffer");
        abort();
    }

    context->device.destroyFence(fence);

    context->device.destroyCommandPool(command_pool);
}