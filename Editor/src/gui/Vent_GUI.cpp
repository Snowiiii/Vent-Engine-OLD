#include "Vent_GUI.hpp"

#include <vk/Vulkan_Base.hpp>

void Vent_GUI::UpdateFontsTexture()
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Updating ImGui Font Textures");

    // TODO: create command buffer
    vk::CommandPool pool;
    vk::CommandBuffer buffer;

    vk::CommandPoolCreateInfo cmd_pool_info(vk::CommandPoolCreateFlagBits::eTransient, context->graphics_queue_index);
    pool = context->device.createCommandPool(cmd_pool_info);

    vk::CommandBufferAllocateInfo cmd_buf_info(pool, vk::CommandBufferLevel::ePrimary, 1);
    buffer = context->device.allocateCommandBuffers(cmd_buf_info).front();

    vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    buffer.begin(begin_info);

    if (!ImGui_ImplVulkan_CreateFontsTexture(buffer))
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed create ImGui Font Texture");
        buffer.end();
        return;
    }

    buffer.end();

    const vk::PipelineStageFlags wait_stage(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    const vk::SubmitInfo info({}, {}, {}, {}, buffer);
    // Submit command buffer to graphics queue
    context->queue.submit(info);
    context->device.waitIdle();

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    if (buffer)
    {
        context->device.freeCommandBuffers(pool, buffer);
        buffer = nullptr;
    }

    if (pool)
    {
        context->device.destroyCommandPool(pool);
        pool = nullptr;
    }
    // TODO: destroy command buffer
}

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void Vent_GUI::createImGuiRenderPass()
{
    vk::AttachmentDescription attachment;
    // Backbuffer format.
    attachment.format = context->swapchain_dimensions.format;
    // Not multisampled.
    attachment.samples = vk::SampleCountFlagBits::e1;
    // When starting the frame, we want tiles to be cleared.
    attachment.loadOp = vk::AttachmentLoadOp::eLoad;
    // When ending the frame, we want tiles to be written out.
    attachment.storeOp = vk::AttachmentStoreOp::eStore;
    // Don't care about stencil since we're not using it.
    attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

    // The image layout will be undefined when the render pass begins.
    attachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    // After the render pass is complete, we will transition to ePresentSrcKHR layout.
    attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // We have one subpass. This subpass has one color attachment.
    // While executing this subpass, the attachment will be in attachment optimal layout.
    vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);

    // We will end up with two transitions.
    // The first one happens right before we start subpass #0, where
    // eUndefined is transitioned into eColorAttachmentOptimal.
    // The final layout in the render pass attachment states ePresentSrcKHR, so we
    // will get a final transition from eColorAttachmentOptimal to ePresetSrcKHR.
    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &color_ref);

    // Create a dependency to external events.
    // We need to wait for the WSI semaphore to signal.
    // Only pipeline stages which depend on eColorAttachmentOutput will
    // actually wait for the semaphore, so we must also wait for that pipeline stage.
    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    // Since we changed the image layout, we need to make the memory visible to
    // color attachment to modify.
    dependency.srcAccessMask = {};
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    // Finally, create the renderpass.
    vk::RenderPassCreateInfo rp_info({}, attachment, subpass, dependency);

    vkAssert(context->device.createRenderPass(rp_info, {}, imgui_render_pass), "Failed to create Render Pass for ImGui");
}

void Vent_GUI::createImGuiDescriporPool()
{
    vk::DescriptorPoolSize pool_sizes[] =
        {
            {vk::DescriptorType::eSampler, 1000},
            {vk::DescriptorType::eCombinedImageSampler, 1000},
            {vk::DescriptorType::eSampledImage, 1000},
            {vk::DescriptorType::eStorageImage, 1000},
            {vk::DescriptorType::eUniformTexelBuffer, 1000},
            {vk::DescriptorType::eStorageTexelBuffer, 1000},
            {vk::DescriptorType::eUniformBuffer, 1000},
            {vk::DescriptorType::eStorageBuffer, 1000},
            {vk::DescriptorType::eUniformBufferDynamic, 1000},
            {vk::DescriptorType::eStorageBufferDynamic, 1000},
            {vk::DescriptorType::eInputAttachment, 1000}};
    const vk::DescriptorPoolCreateInfo descriptor_pool_create_info({}, 1000 * IM_ARRAYSIZE(pool_sizes), pool_sizes);

    vkAssert(context->device.createDescriptorPool(descriptor_pool_create_info, {}, imgui_descriptor_pool), "Failed to create DescriporPool for ImGui");
}

Vent_GUI::Vent_GUI(const Vent_Window &window)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "initializing ImGui");
    IMGUI_CHECKVERSION();
    m_context = ImGui::CreateContext();
    if (!m_context)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to Create ImGui Context");
        return;
    }

    ImGui::StyleColorsDark();

    if (!ImGui_ImplSDL2_InitForVulkan(window.handle))
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to initializing ImGui SDL");
        return;
    }

    this->createImGuiDescriporPool();
    this->createImGuiRenderPass();

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "initializing ImGui VK Info ");
    ImGui_ImplVulkan_InitInfo vulkanInitInfo = {};
    vulkanInitInfo.Instance = static_cast<VkInstance>(context->instance);
    vulkanInitInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(context->gpu);
    vulkanInitInfo.Device = static_cast<VkDevice>(context->device);
    vulkanInitInfo.QueueFamily = static_cast<uint32_t>(context->graphics_queue_index);
    vulkanInitInfo.Queue = static_cast<VkQueue>(context->queue);
    vulkanInitInfo.PipelineCache = VK_NULL_HANDLE;
    vulkanInitInfo.DescriptorPool = static_cast<VkDescriptorPool>(imgui_descriptor_pool);
    vulkanInitInfo.Subpass = 0;
    vulkanInitInfo.MinImageCount = 2;
    vulkanInitInfo.ImageCount = context->swapchain_image_views.size();
    vulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    vulkanInitInfo.Allocator = nullptr;
    vulkanInitInfo.CheckVkResultFn = check_vk_result;

    if (!ImGui_ImplVulkan_Init(&vulkanInitInfo, imgui_render_pass))
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to initializing ImGui Vulkan");
        return;
    }

    UpdateFontsTexture();
}

void Vent_GUI::handleSDLEvent(SDL_Event event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void Vent_GUI::update(const vk::CommandBuffer &buffer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);
}

Vent_GUI::~Vent_GUI()
{
    if (imgui_descriptor_pool)
    {
        context->device.destroyDescriptorPool(imgui_descriptor_pool);
    }

    if (imgui_render_pass)
    {
        context->device.destroyRenderPass(imgui_render_pass);
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext(m_context);
}