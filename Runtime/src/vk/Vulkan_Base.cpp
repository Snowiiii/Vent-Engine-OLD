#include "Vulkan_Base.hpp"

#include "uniform/Vulkan_3D_Unifrom.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

VulkanContext *context = new VulkanContext;

#ifdef VALIDATION_LAYERS
/// @brief A debug callback called from Vulkan validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT type,
													 uint64_t object, size_t location, int32_t message_code,
													 const char *layer_prefix, const char *message, void *user_data)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Validation Layer: Error: %s: %s", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Validation Layer: Warning: %s: %s", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Validation Layer: Performance warning: %s: %s", layer_prefix, message);
	}
	else
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Validation Layer: Information: %s: %s", layer_prefix, message);
	}
	return VK_FALSE;
}
#endif

void VKBase::createDepthFormat()
{
	const std::vector<vk::Format> &depth_format_priority_list = {
		vk::Format::eD32Sfloat,
		vk::Format::eD24UnormS8Uint,
		vk::Format::eD16Unorm};

	vk::Format finalFormat = vk::Format::eUndefined;

	for (auto &format : depth_format_priority_list)
	{
		vk::FormatProperties properties = context->gpu.getFormatProperties(format);

		// Format must support depth stencil attachment for optimal tiling
		if (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		{
			finalFormat = format;
			break;
		}
	}

	if (finalFormat != vk::Format::eUndefined)
	{
		context->depthFormat = finalFormat;
		return;
	}

	throw std::runtime_error("No suitable depth format could be determined");
}

bool VKBase::is_extension_supported(std::string const &requested_extension) const
{
	return std::find_if(device_extensions.begin(),
						device_extensions.end(),
						[requested_extension](auto &device_extension)
						{ return std::strcmp(device_extension.extensionName, requested_extension.c_str()) == 0; }) != device_extensions.end();
}

bool validate_extensions(const std::vector<const char *> &required,
						 const std::vector<vk::ExtensionProperties> &available)
{
	// inner find_if gives true if the extension was not found
	// outer find_if gives true if none of the extensions were not found, that is if all extensions were found
	return std::find_if(required.begin(),
						required.end(),
						[&available](auto extension)
						{
							return std::find_if(available.begin(),
												available.end(),
												[&extension](auto const &ep)
												{
													return strcmp(ep.extensionName, extension) == 0;
												}) == available.end();
						}) == required.end();
}

bool validate_layers(const std::vector<const char *> &required,
					 const std::vector<vk::LayerProperties> &available)
{
	// inner find_if returns true if the layer was not found
	// outer find_if returns iterator to the not found layer, if any
	auto requiredButNotFoundIt = std::find_if(required.begin(),
											  required.end(),
											  [&available](auto layer)
											  {
												  return std::find_if(available.begin(),
																	  available.end(),
																	  [&layer](auto const &lp)
																	  {
																		  return strcmp(lp.layerName, layer) == 0;
																	  }) == available.end();
											  });
	if (requiredButNotFoundIt != required.end())
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Validation Layer %s not found", *requiredButNotFoundIt);
	}
	return (requiredButNotFoundIt == required.end());
}

std::vector<const char *> get_optimal_validation_layers(const std::vector<vk::LayerProperties> &supported_instance_layers)
{
	std::vector<std::vector<const char *>> validation_layer_priority_list =
		{
			// The preferred validation layer is "VK_LAYER_KHRONOS_validation"
			{"VK_LAYER_KHRONOS_validation"},

			// Otherwise we fallback to using the LunarG meta layer
			{"VK_LAYER_LUNARG_standard_validation"},

			// Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
			{
				"VK_LAYER_GOOGLE_threading",
				"VK_LAYER_LUNARG_parameter_validation",
				"VK_LAYER_LUNARG_object_tracker",
				"VK_LAYER_LUNARG_core_validation",
				"VK_LAYER_GOOGLE_unique_objects",
			},

			// Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
			{"VK_LAYER_LUNARG_core_validation"}};

	for (auto &validation_layers : validation_layer_priority_list)
	{
		if (validate_layers(validation_layers, supported_instance_layers))
		{
			return validation_layers;
		}

		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Couldn't enable validation layers (see log for error) - falling back");
	}

	// Else return nothing
	return {};
}

void VKBase::createInstance()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Creating VKInstance");

	static vk::DynamicLoader dl;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
		dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	const std::vector<vk::ExtensionProperties> instance_extensions = vk::enumerateInstanceExtensionProperties();

	uint32_t instanceExtensionCount;
	if (!SDL_Vulkan_GetInstanceExtensions(window.handle, &instanceExtensionCount, nullptr))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Failed to Vulkan Instance Extensions count from SDL");
		return;
	}
	const char **enabledInstanceExtensions = new const char *[instanceExtensionCount];
	if (!SDL_Vulkan_GetInstanceExtensions(window.handle, &instanceExtensionCount, enabledInstanceExtensions))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Failed to Vulkan Instance Extensions from SDL");
		return;
	}

	std::vector<const char *> active_instance_extensions(enabledInstanceExtensions, enabledInstanceExtensions + instanceExtensionCount);

#ifdef VALIDATION_LAYERS
	active_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	if (!validate_extensions(active_instance_extensions, instance_extensions))
	{
		throw std::runtime_error("Required instance extensions are missing.");
	}

	const std::vector<vk::LayerProperties> supported_validation_layers = vk::enumerateInstanceLayerProperties();

	std::vector<const char *> requested_validation_layers;

#ifdef VALIDATION_LAYERS
	// Determine the optimal validation layers to enable that are necessary for useful debugging
	const std::vector<const char *> optimal_validation_layers = get_optimal_validation_layers(supported_validation_layers);
	requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());
#endif

	if (validate_layers(requested_validation_layers, supported_validation_layers))
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Enabled Validation Layers:");
		for (const auto &layer : requested_validation_layers)
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "	\t%s", layer);
		}
	}
	else
	{
		throw std::runtime_error("Required validation layers are missing.");
	}

	// vulkan application info
	const vk::ApplicationInfo app("Vent-Engine",
								  VK_MAKE_VERSION(1, 0, 0),
								  "Vent-Engine",
								  VK_MAKE_VERSION(1, 0, 0),
								  VK_API_VERSION_1_3);

	// vulkan instance info
	vk::InstanceCreateInfo instance_info({}, &app, requested_validation_layers, active_instance_extensions);

#ifdef VALIDATION_LAYERS
	const vk::DebugReportCallbackCreateInfoEXT debug_report_create_info(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning, debug_callback);

	instance_info.pNext = &debug_report_create_info;

#endif

	vkAssert(vk::createInstance(&instance_info, {}, &context->instance), "Failed to create Vulkan instance");

	VULKAN_HPP_DEFAULT_DISPATCHER.init(context->instance);

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
	// we need some additional initializing for this platform!
	if (volkInitialize())
	{
		throw std::runtime_error("Failed to initialize volk.");
	}
	volkLoadInstance(context->instance);
#endif

#ifdef VALIDATION_LAYERS
	context->debug_callback = context->instance.createDebugReportCallbackEXT(debug_report_create_info);
#endif
}

void VKBase::selectPhysicalDevice()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Selecting Physical GPU");
	const std::vector<vk::PhysicalDevice> gpus = context->instance.enumeratePhysicalDevices();

	for (size_t i = 0; i < gpus.size() && (context->graphics_queue_index < 0); i++)
	{
		context->gpu = gpus[i];

		const std::vector<vk::QueueFamilyProperties2> queue_family_properties = context->gpu.getQueueFamilyProperties2();

		if (queue_family_properties.empty())
		{
			throw std::runtime_error("No queue family found.");
		}

		if (context->surface)
		{
			context->instance.destroySurfaceKHR(context->surface);
		}
		context->surface = window.createSurface(context->instance);
		if (!context->surface)
			throw std::runtime_error("Failed to create window surface.");

		for (uint32_t j = 0; j < static_cast<uint32_t>(queue_family_properties.size()); j++)
		{
			vk::Bool32 supports_present = context->gpu.getSurfaceSupportKHR(j, context->surface);

			// Find a queue family which supports graphics and presentation.
			if ((queue_family_properties[j].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) && supports_present)
			{
				context->graphics_queue_index = j;
				break;
			}
		}
	}

	if (context->graphics_queue_index < 0)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Did not find suitable queue which supports graphics and presentation.");
	}
}

void VKBase::createDevice(const std::vector<const char *> &required_device_extensions)
{
	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Creating VK Device");
	device_extensions = context->gpu.enumerateDeviceExtensionProperties();

	if (!validate_extensions(required_device_extensions, device_extensions))
	{
		throw std::runtime_error("Required device extensions are missing, will try without.");
	}

	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Enables GPU Features");

	vk::PhysicalDeviceFeatures features;
	if (context->gpu.getFeatures().samplerAnisotropy)
	{
		features.samplerAnisotropy = true;
	}

	float queue_priority = 1.0f;

	// Create one queue
	vk::DeviceQueueCreateInfo queue_info({}, context->graphics_queue_index, 1, &queue_priority);

	const vk::DeviceCreateInfo device_info({}, queue_info, {}, required_device_extensions, &features);

	vkAssert(context->gpu.createDevice(&device_info, {}, &context->device), "Failed to Create Vulkan Device");
	// initialize function pointers for device
	VULKAN_HPP_DEFAULT_DISPATCHER.init(context->device);

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
	volkLoadDevice(context->device);
#endif

	context->queue = context->device.getQueue(context->graphics_queue_index, 0);
}

void VKBase::createAllocator()
{
	SDL_LogDebug(SDL_LOG_CATEGORY_RENDER, "Creating VK Memory Allocator");
	static vk::DynamicLoader dl;

	VmaVulkanFunctions vma_vulkan_func{};
	vma_vulkan_func.vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	vma_vulkan_func.vkGetDeviceProcAddr = dl.getProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr");

	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;
	allocator_info.physicalDevice = static_cast<VkPhysicalDevice>(context->gpu);
	allocator_info.device = static_cast<VkDevice>(context->device);
	allocator_info.instance = static_cast<VkInstance>(context->instance);

	allocator_info.pVulkanFunctions = &vma_vulkan_func;

	vkAssert(static_cast<VkResult>(vmaCreateAllocator(&allocator_info, &context->memory_allocator)), "Failed to create VMA Allocator");
}

void VKBase::createCommandPool()
{
	vk::CommandPoolCreateInfo cmd_pool_info(vk::CommandPoolCreateFlagBits::eTransient, context->graphics_queue_index);
	vkAssert(context->device.createCommandPool(&cmd_pool_info,{}, &context->command_pool), "Failed to create Command Pool");
}

void VKBase::createDescriptorPool()
{
	const std::array<vk::DescriptorPoolSize, 2> pool_sizes = {{{vk::DescriptorType::eUniformBuffer, 1000}, {vk::DescriptorType::eCombinedImageSampler, 1000}}};

	const vk::DescriptorPoolCreateInfo descriptor_pool_create_info({}, 2, pool_sizes);

	vkAssert(context->device.createDescriptorPool(&descriptor_pool_create_info,{}, &context->descriptor_pool), "Failed to create DescriptorPool");
}

void VKBase::createDescriptorSetLayoutBinding()
{
	const std::array<vk::DescriptorSetLayoutBinding, 2> set_layout_bindings = {
		{{0, vk::DescriptorType::eUniformBufferDynamic, 1, vk::ShaderStageFlagBits::eVertex},
		 {1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}}};

	vk::DescriptorSetLayoutCreateInfo descriptor_layout({}, set_layout_bindings);

	context->descriptor_set_layout = context->device.createDescriptorSetLayout(descriptor_layout);

	vk::PushConstantRange pushConstant(vk::ShaderStageFlagBits::eVertex, 0, sizeof(Ubo));

#if defined(ANDROID)
	vk::PipelineLayoutCreateInfo pipeline_layout_create_info({}, 1, &context->descriptor_set_layout, pushConstant);
#else
	vk::PipelineLayoutCreateInfo pipeline_layout_create_info({}, context->descriptor_set_layout, pushConstant);
#endif

	vkAssert(context->device.createPipelineLayout(&pipeline_layout_create_info, 0,context->pipeline_layout), "Failed to create Pipeline Layout");
}

void VKBase::initVulkan()
{
	this->createInstance();
	this->selectPhysicalDevice();

	context->swapchain_dimensions.width = window.width;
	context->swapchain_dimensions.height = window.height;

	this->createDevice({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
	this->createAllocator();
	this->createCommandPool();
	this->createDescriptorPool();
	this->createDescriptorSetLayoutBinding();
	this->createDepthFormat();
}

void VKBase::shutdownVulkan()
{
	if (context->descriptor_pool)
	{
		context->device.destroyDescriptorPool(context->descriptor_pool);
	}

	if (context->command_pool)
	{
		context->device.destroyCommandPool(context->command_pool);
	}

	if (context->memory_allocator)
	{
		VmaTotalStatistics stats;
		vmaCalculateStatistics(context->memory_allocator, &stats);
		const VkDeviceSize bytes = stats.total.statistics.allocationBytes;

		if (bytes > 0)
		{
			SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Total device memory leaked: %lu bytes.", bytes);
		}

		vmaDestroyAllocator(context->memory_allocator);
		context->memory_allocator = nullptr;
	}

	if (context->descriptor_set_layout)
	{
		context->device.destroyDescriptorSetLayout(context->descriptor_set_layout);
		context->descriptor_set_layout = nullptr;
	}

	if (context->pipeline_layout)
	{
		context->device.destroyPipelineLayout(context->pipeline_layout);
		context->pipeline_layout = nullptr;
	}

	if (context->surface)
	{
		context->instance.destroySurfaceKHR(context->surface);
		context->surface = nullptr;
	}

	if (context->debug_callback)
	{
		context->instance.destroyDebugReportCallbackEXT(context->debug_callback);
		context->debug_callback = nullptr;
	}

	if (context->device)
	{
		context->device.destroy();
		context->device = nullptr;
	}

	if (context->instance)
	{
		context->instance.destroy();
		context->instance = nullptr;
	}

	delete context;
}

VKBase::VKBase(const Vent_Window &pwindow) : window(pwindow) {}
