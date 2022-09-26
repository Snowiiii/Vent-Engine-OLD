#include "Renderer.hpp"

Renderer::Renderer()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Loading VK Renderer");
	vkbase.initVulkan();

	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Creating VK Swapchain");
	vkbase.createSwapchain();

	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Creating VK RenderPass");
	vkbase.createRenderPass();

	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Loading Shader Uniform");
	this->loadUniform();

	objectRenderer = std::make_unique<ObjectRenderer>();

	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Loading Models");
	this->loadModels();

	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Creating VK Pipeline");
	vkbase.createPipeline("assets/shaders/model.vert.spv", "assets/shaders/model.frag.spv");

	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Init FrameBuffers");
	this->init_framebuffers();
}

void Renderer::loadModels()
{
	vk::DescriptorBufferInfo buffer_descriptor(uniform->getBuffer(), 0, sizeof(Vulkan_3D_Unifrom::ubo_vs));
	std::unique_ptr<Vulkan_Mesh> model = std::make_unique<Vulkan_Mesh>("assets/meshes/dragon.obj");
	model->setTexture(buffer_descriptor, "assets/textures/texture.png");

	objectRenderer->addModel(model);
}

void Renderer::loadUniform()
{
	uniform = std::make_unique<Vulkan_3D_Unifrom>(camera);
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
	this->teardown_framebuffers();

	vkbase.createSwapchain();
	this->init_framebuffers();
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
	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Shutting down Vulkan");
	context->device.waitIdle();

	this->teardown_framebuffers();

	if (objectRenderer)
	{
		objectRenderer.reset();
	}

	if (uniform)
	{
		uniform.reset();
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

vk::Result Renderer::present_image(uint32_t &index)
{
	vk::PresentInfoKHR present(context->per_frame[index].swapchain_release_semaphore, context->swapchain, index);
	// Present swapchain image
	return context->queue.presentKHR(present);
}

void Renderer::update(float delta)
{
	uint32_t index;

	auto res = this->acquire_next_image(index);

	// Handle outdated error in acquire.
	if (res == vk::Result::eSuboptimalKHR || res == vk::Result::eErrorOutOfDateKHR)
	{
		this->resize(context->swapchain_dimensions.width, context->swapchain_dimensions.height);
		res = acquire_next_image(index);
	}

	if (res != vk::Result::eSuccess)
	{
		context->queue.waitIdle();
		return;
	}

	this->render(index);
	res = this->present_image(index);

	// Handle Outdated error in present.
	if (res == vk::Result::eSuboptimalKHR || res == vk::Result::eErrorOutOfDateKHR)
	{
		this->resize(context->swapchain_dimensions.width, context->swapchain_dimensions.height);
	}
	else if (res != vk::Result::eSuccess)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Failed to present swapchain image.");
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

void Renderer::render(uint32_t &swapchain_index)
{
	// Only update if needed
	uniform->updateViewMatrix(camera);

	vk::Framebuffer framebuffer = context->swapchain_framebuffers[swapchain_index];

	vk::CommandBuffer cmd = context->per_frame[swapchain_index].primary_command_buffer;

	vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	cmd.begin(begin_info);

	vk::ClearValue clear_values[2];
	clear_values[0].color = vk::ClearColorValue(std::array<float, 4>({{0.1f, 0.0f, 0.2f, 1.0f}}));
	clear_values[1].depthStencil = vk::ClearDepthStencilValue(0.0f, 0);

	vk::RenderPassBeginInfo rp_begin(context->render_pass, framebuffer, {{0, 0}, {context->swapchain_dimensions.width, context->swapchain_dimensions.height}},
									 clear_values);

	cmd.beginRenderPass(rp_begin, vk::SubpassContents::eInline);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, context->pipeline);

	vk::Viewport vp(0.0f, 0.0f, static_cast<float>(context->swapchain_dimensions.width), static_cast<float>(context->swapchain_dimensions.height), 0.0f, 1.0f);
	// Set viewport dynamically
	cmd.setViewport(0, vp);

	const vk::Rect2D scissor({0, 0}, {context->swapchain_dimensions.width, context->swapchain_dimensions.height});

	cmd.setScissor(0, scissor);

	objectRenderer->render(cmd, uniform);

	cmd.endRenderPass();

	cmd.end();

	uniform->uniform_buffer_general->flush();

	if (!context->per_frame[swapchain_index].swapchain_release_semaphore)
	{
		context->per_frame[swapchain_index].swapchain_release_semaphore = context->device.createSemaphore({});
	}

	const vk::PipelineStageFlags wait_stage(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	const vk::SubmitInfo info(context->per_frame[swapchain_index].swapchain_acquire_semaphore, wait_stage, cmd,
							  context->per_frame[swapchain_index].swapchain_release_semaphore);
	// Submit command buffer to graphics queue
	context->queue.submit(info, context->per_frame[swapchain_index].queue_submit_fence);
}
