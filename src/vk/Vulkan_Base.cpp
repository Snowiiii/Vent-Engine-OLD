#include "Vulkan_Base.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

VulkanContext* context = new VulkanContext;

#ifdef VALIDATION_LAYERS
/// @brief A debug callback called from Vulkan validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT type,
													 uint64_t object, size_t location, int32_t message_code,
													 const char *layer_prefix, const char *message, void *user_data)
{
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		spdlog::error("Validation Layer: Error: {}: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		spdlog::warn("Validation Layer: Warning: {}: {}", layer_prefix, message);
	}
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		spdlog::debug("Validation Layer: Performance warning: {}: {}", layer_prefix, message);
	}
	else
	{
		spdlog::info("Validation Layer: Information: {}: {}", layer_prefix, message);
	}
	return VK_FALSE;
}
#endif

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
		spdlog::error("Validation Layer {} not found", *requiredButNotFoundIt);
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

		spdlog::warn("Couldn't enable validation layers (see log for error) - falling back");
	}

	// Else return nothing
	return {};
}

void VKBase::createInstance()
{
	spdlog::debug("Creating VKInstance");

	static vk::DynamicLoader dl;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
		dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	std::vector<vk::ExtensionProperties> instance_extensions = vk::enumerateInstanceExtensionProperties();

	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> active_instance_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef VALIDATION_LAYERS
	active_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	active_instance_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
	active_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
	active_instance_extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	active_instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	active_instance_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	active_instance_extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DISPLAY_KHR)
	active_instance_extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#endif

	if (!validate_extensions(active_instance_extensions, instance_extensions))
	{
		throw std::runtime_error("Required instance extensions are missing.");
	}

	std::vector<vk::LayerProperties> supported_validation_layers = vk::enumerateInstanceLayerProperties();

	std::vector<const char *> requested_validation_layers;

#ifdef VALIDATION_LAYERS
	// Determine the optimal validation layers to enable that are necessary for useful debugging
	std::vector<const char *> optimal_validation_layers = get_optimal_validation_layers(supported_validation_layers);
	requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());
#endif

	if (validate_layers(requested_validation_layers, supported_validation_layers))
	{
		spdlog::info("Enabled Validation Layers:");
		for (const auto &layer : requested_validation_layers)
		{
			spdlog::info("	\t{}", layer);
		}
	}
	else
	{
		throw std::runtime_error("Required validation layers are missing.");
	}

	// vulkan application info
	vk::ApplicationInfo app("Vent-Engine",
							VK_MAKE_VERSION(1, 0, 0),
							"Vent-Engine",
							VK_MAKE_VERSION(1, 0, 0),
							VK_API_VERSION_1_2);

	// vulkan instance info
	vk::InstanceCreateInfo instance_info({}, &app, requested_validation_layers, active_instance_extensions);

#ifdef VALIDATION_LAYERS
	vk::DebugReportCallbackCreateInfoEXT debug_report_create_info(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning, debug_callback);

	instance_info.pNext = &debug_report_create_info;

#endif

	context->instance = vk::createInstance(instance_info);

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
	spdlog::debug("Selecting Physical GPU");
	std::vector<vk::PhysicalDevice> gpus = context->instance.enumeratePhysicalDevices();

	for (size_t i = 0; i < gpus.size() && (context->graphics_queue_index < 0); i++)
	{
		context->gpu = gpus[i];

		std::vector<vk::QueueFamilyProperties> queue_family_properties = context->gpu.getQueueFamilyProperties();

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
			if ((queue_family_properties[j].queueFlags & vk::QueueFlagBits::eGraphics) && supports_present)
			{
				context->graphics_queue_index = j;
				break;
			}
		}
	}

	if (context->graphics_queue_index < 0)
	{
		spdlog::critical("Did not find suitable queue which supports graphics and presentation.");
	}
}

void VKBase::createDevice(const std::vector<const char *> &required_device_extensions)
{
	spdlog::debug("Creating VK Device");
	device_extensions = context->gpu.enumerateDeviceExtensionProperties();

	if (!validate_extensions(required_device_extensions, device_extensions))
	{
		throw std::runtime_error("Required device extensions are missing, will try without.");
	}

	float queue_priority = 1.0f;

	// Create one queue
	vk::DeviceQueueCreateInfo queue_info({}, context->graphics_queue_index, 1, &queue_priority);

	vk::DeviceCreateInfo device_info({}, queue_info, {}, required_device_extensions);

	context->device = context->gpu.createDevice(device_info);
	// initialize function pointers for device
	VULKAN_HPP_DEFAULT_DISPATCHER.init(context->device);

	//	volkLoadDevice(context->device);

	context->queue = context->device.getQueue(context->graphics_queue_index, 0);
}

void VKBase::createAllocator()
{
	spdlog::debug("Creating VK Memory Allocator");
	static vk::DynamicLoader dl;

	VmaVulkanFunctions vma_vulkan_func{};

	vma_vulkan_func.vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	vma_vulkan_func.vkGetDeviceProcAddr = dl.getProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr");

	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.vulkanApiVersion = VK_API_VERSION_1_2;
	allocator_info.physicalDevice = static_cast<VkPhysicalDevice>(context->gpu);
	allocator_info.device = static_cast<VkDevice>(context->device);
	allocator_info.instance = static_cast<VkInstance>(context->instance);

	bool can_get_memory_requirements = is_extension_supported(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
	bool has_dedicated_allocation = is_extension_supported(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

	// if (can_get_memory_requirements && has_dedicated_allocation)
	// {
	// 	allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	// 	vma_vulkan_func.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	// 	vma_vulkan_func.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
	// }

	allocator_info.pVulkanFunctions = &vma_vulkan_func;

	auto result = vmaCreateAllocator(&allocator_info, &context->memory_allocator);
}

void VKBase::initVulkan()
{
	createInstance();
	selectPhysicalDevice();

	const auto &extent = window.getExtent();
	context->swapchain_dimensions.width = extent.width;
	context->swapchain_dimensions.height = extent.height;

	createDevice({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
	createAllocator();
}

void VKBase::shutdownVulkan()
{
	if (context->memory_allocator != VK_NULL_HANDLE)
	{
		VmaTotalStatistics stats;
		vmaCalculateStatistics(context->memory_allocator, &stats);

		spdlog::warn("Total device memory leaked: {} bytes.", stats.total.statistics.allocationBytes);

		vmaDestroyAllocator(context->memory_allocator);
	}

	if (context->device)
	{
		context->device.destroy();
		context->device = nullptr;
	}

	context->instance.destroy();
}

VKBase::VKBase(const Vent_Window &window) : window(window) {}
