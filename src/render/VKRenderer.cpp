#include "Renderer.hpp"

Renderer::Renderer()
{
	spdlog::debug("Loading VK Renderer");
	vkbase.initVulkan();
	spdlog::debug("Creating VK Swapchain");
	vkbase.createSwapchain();
	spdlog::debug("Creating VK RenderPass");
	vkbase.createRenderPass();
	spdlog::debug("Loading Models");
	this->loadModels();
	spdlog::debug("Creating VK Pipeline");
	vkbase.createPipeline("shaders/spv/color.vert.spv", "shaders/spv/color.frag.spv");

	spdlog::debug("Init FrameBuffers");
	this->init_framebuffers();
}

void Renderer::loadModels()
{
	std::vector<Vertex> vertices{  {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};
	std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

	model = std::make_unique<Vulkan_Model>(vertices, indices);
}

bool Renderer::resize(const uint32_t, const uint32_t)
{
	if (!context->device)
	{
		return false;
	}

	vk::SurfaceCapabilitiesKHR surface_properties = context->gpu.getSurfaceCapabilitiesKHR(context->surface);

	// Only rebuild the swapchain if the dimensions have changed
	if (surface_properties.currentExtent.width == context->swapchain_dimensions.width &&
		surface_properties.currentExtent.height == context->swapchain_dimensions.height)
	{
		return false;
	}

	context->device.waitIdle();
	teardown_framebuffers();

	vkbase.createSwapchain();
	init_framebuffers();
	return true;
}

void Renderer::init_framebuffers()
{
	vk::Device device = context->device;

	// Create framebuffer for each swapchain image view
	for (auto &image_view : context->swapchain_image_views)
	{
		// Build the framebuffer.
		vk::FramebufferCreateInfo fb_info({}, context->render_pass, image_view, context->swapchain_dimensions.width, context->swapchain_dimensions.height, 1);

		context->swapchain_framebuffers.push_back(device.createFramebuffer(fb_info));
	}
}

void Renderer::teardown_framebuffers()
{
	// Wait until device is idle before teardown.
	context->queue.waitIdle();

	for (auto &framebuffer : context->swapchain_framebuffers)
	{
		context->device.destroyFramebuffer(framebuffer);
	}

	context->swapchain_framebuffers.clear();
}

Renderer::~Renderer()
{
	spdlog::debug("Shutting down Vulkan");
	context->device.waitIdle();

	teardown_framebuffers();

	if (model)
	{
		model.reset();
	}
	

	for (auto &per_frame : context->per_frame)
	{
		vkbase.teardown_per_frame(per_frame);
	}

	context->per_frame.clear();

	for (auto semaphore : context->recycled_semaphores)
	{
		context->device.destroySemaphore(semaphore);
	}

	vkbase.destroyPipeline();

	vkbase.destroyRenderpass();

	vkbase.destroySwapchain();

	vkbase.shutdownVulkan();
}

vk::Result Renderer::present_image(uint32_t index)
{
	vk::PresentInfoKHR present(context->per_frame[index].swapchain_release_semaphore, context->swapchain, index);
	// Present swapchain image
	return context->queue.presentKHR(present);
}

void Renderer::update(float delta)
{
	uint32_t index;

	auto res = acquire_next_image(index);

	// Handle outdated error in acquire.
	if (res == vk::Result::eSuboptimalKHR || res == vk::Result::eErrorOutOfDateKHR)
	{
		resize(context->swapchain_dimensions.width, context->swapchain_dimensions.height);
		res = acquire_next_image(index);
	}

	if (res != vk::Result::eSuccess)
	{
		context->queue.waitIdle();
		return;
	}

	render(index);
	res = present_image(index);

	// Handle Outdated error in present.
	if (res == vk::Result::eSuboptimalKHR || res == vk::Result::eErrorOutOfDateKHR)
	{
		resize(context->swapchain_dimensions.width, context->swapchain_dimensions.height);
	}
	else if (res != vk::Result::eSuccess)
	{
		spdlog::warn("Failed to present swapchain image.");
	}
}

vk::Result Renderer::acquire_next_image(uint32_t &image)
{
	vk::Semaphore acquire_semaphore;
	if (context->recycled_semaphores.empty())
	{
		acquire_semaphore = context->device.createSemaphore({});
	}
	else
	{
		acquire_semaphore = context->recycled_semaphores.back();
		context->recycled_semaphores.pop_back();
	}

	vk::Result res;
	std::tie(res, image) = context->device.acquireNextImageKHR(context->swapchain, UINT64_MAX, acquire_semaphore);

	if (res != vk::Result::eSuccess)
	{
		context->recycled_semaphores.push_back(acquire_semaphore);
		return res;
	}

	// If we have outstanding fences for this swapchain image, wait for them to complete first.
	// After begin frame returns, it is safe to reuse or delete resources which
	// were used previously.
	//
	// We wait for fences which completes N frames earlier, so we do not stall,
	// waiting for all GPU work to complete before this returns.
	// Normally, this doesn't really block at all,
	// since we're waiting for old frames to have been completed, but just in case.
	if (context->per_frame[image].queue_submit_fence)
	{
		auto result = context->device.waitForFences(context->per_frame[image].queue_submit_fence, true, UINT64_MAX);
		if (result != vk::Result::eSuccess)
			return result;
		context->device.resetFences(context->per_frame[image].queue_submit_fence);
	}

	if (context->per_frame[image].primary_command_pool)
	{
		context->device.resetCommandPool(context->per_frame[image].primary_command_pool);
	}

	// Recycle the old semaphore back into the semaphore manager.
	vk::Semaphore old_semaphore = context->per_frame[image].swapchain_acquire_semaphore;

	if (old_semaphore)
	{
		context->recycled_semaphores.push_back(old_semaphore);
	}

	context->per_frame[image].swapchain_acquire_semaphore = acquire_semaphore;

	return vk::Result::eSuccess;
}

void Renderer::render(uint32_t swapchain_index)
{
	static float greenChannel = 0.0f;
	greenChannel += 0.01f;
	if (greenChannel > 1.0f)
		greenChannel = 0.0f;

	vk::Framebuffer framebuffer = context->swapchain_framebuffers[swapchain_index];

	vk::CommandBuffer cmd = context->per_frame[swapchain_index].primary_command_buffer;

	vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	cmd.begin(begin_info);

	vk::ClearValue clear_value;
	clear_value.color = vk::ClearColorValue(std::array<float, 4>({{0.1f, greenChannel, 0.2f, 1.0f}}));

	vk::RenderPassBeginInfo rp_begin(context->render_pass, framebuffer, {{0, 0}, {context->swapchain_dimensions.width, context->swapchain_dimensions.height}},
									 clear_value);

	cmd.beginRenderPass(rp_begin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, context->pipeline);

	vk::Viewport vp(0.0f, 0.0f, static_cast<float>(context->swapchain_dimensions.width), static_cast<float>(context->swapchain_dimensions.height), 0.0f, 1.0f);
	// Set viewport dynamically
	cmd.setViewport(0, vp);

	const vk::Rect2D scissor({0, 0}, {context->swapchain_dimensions.width, context->swapchain_dimensions.height});

	cmd.setScissor(0, scissor);

	model->bind(cmd);
	model->draw(cmd);

	cmd.endRenderPass();

	cmd.end();

	if (!context->per_frame[swapchain_index].swapchain_release_semaphore)
	{
		context->per_frame[swapchain_index].swapchain_release_semaphore = context->device.createSemaphore({});
	}

	vk::PipelineStageFlags wait_stage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	vk::SubmitInfo info(context->per_frame[swapchain_index].swapchain_acquire_semaphore, wait_stage, cmd,
						context->per_frame[swapchain_index].swapchain_release_semaphore);
	// Submit command buffer to graphics queue
	context->queue.submit(info, context->per_frame[swapchain_index].queue_submit_fence);
}
