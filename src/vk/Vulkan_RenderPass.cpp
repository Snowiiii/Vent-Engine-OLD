#include "Vulkan_Base.hpp"

void VKBase::createRenderPass()
{
	vk::AttachmentDescription attachments[2];
	// Backbuffer format.
	attachments[0].format = context->swapchain_dimensions.format;
	// Not multisampled.
	attachments[0].samples = vk::SampleCountFlagBits::e1;
	// When starting the frame, we want tiles to be cleared.
	attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
	// When ending the frame, we want tiles to be written out.
	attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
	// Don't care about stencil since we're not using it.
	attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	// The image layout will be undefined when the render pass begins.
	attachments[0].initialLayout = vk::ImageLayout::eUndefined;
	// After the render pass is complete, we will transition to ePresentSrcKHR layout.
	attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

	// Depth

	// Backbuffer format.
	attachments[1].format = vk::Format::eD32Sfloat;
	// Not multisampled.
	attachments[1].samples = vk::SampleCountFlagBits::e1;
	// When starting the frame, we want tiles to be cleared.
	attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
	// When ending the frame, we want tiles to be written out.
	attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
	// Don't care about stencil since we're not using it.
	attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	// The image layout will be undefined when the render pass begins.
	attachments[1].initialLayout = vk::ImageLayout::eUndefined;
	// After the render pass is complete, we will transition to ePresentSrcKHR layout.
	attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	// We have one subpass. This subpass has one color attachment.
	// While executing this subpass, the attachment will be in attachment optimal layout.
	vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depth_ref(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// We will end up with two transitions.
	// The first one happens right before we start subpass #0, where
	// eUndefined is transitioned into eColorAttachmentOptimal.
	// The final layout in the render pass attachment states ePresentSrcKHR, so we
	// will get a final transition from eColorAttachmentOptimal to ePresetSrcKHR.
	vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {}, {}, 1, &color_ref, {}, &depth_ref);

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
	vk::RenderPassCreateInfo rp_info({}, attachments, subpass, dependency);

	context->render_pass = context->device.createRenderPass(rp_info);
}

void VKBase::destroyRenderpass()
{
	if (context->render_pass)
	{
		context->device.destroyRenderPass(context->render_pass);
		context->render_pass = nullptr;
	}
}
