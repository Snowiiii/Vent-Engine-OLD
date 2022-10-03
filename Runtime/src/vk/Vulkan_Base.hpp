#pragma once

#include "../Vent_Window.hpp"
#include <vk_mem_alloc.h>
#include <volk.h>

#include "../objects/Camera.hpp"
#include "../objects/GameObject.hpp"

#include <glm/glm.hpp>

#include <cassert>
#include <vector>

#ifndef NDEBUG
#define VALIDATION_LAYERS
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void vkAssert(vk::Result result, const char *description)
{
    if (result != vk::Result::eSuccess)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Vulkan Assert failed,Result %s, %s", to_string(result).c_str(), description);
    }
}

const std::string to_string(vk::Result result)
{
    switch ((VkResult)result)
    {
#define STR(r)   \
    case VK_##r: \
        return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

struct Ubo;

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uv; // Tex Coords
    glm::vec3 normal;

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
    static vk::VertexInputBindingDescription getBindingDescription();
};

struct Texture
{
    vk::Sampler sampler;

    vk::Image image;

    VmaAllocation allocation;

    vk::ImageLayout image_layout;

    vk::ImageView view;

    vk::Extent2D extent;

    uint32_t mip_levels;
};

struct SwapchainDimensions
{
    /// Width of the swapchain.
    uint32_t width = 0;

    /// Height of the swapchain.
    uint32_t height = 0;

    /// Pixel format of the swapchain.
    vk::Format format = vk::Format::eUndefined;
};

struct PerFrame
{
    vk::Device device;

    vk::Fence queue_submit_fence;

    vk::CommandPool primary_command_pool;

    vk::CommandBuffer primary_command_buffer;

    vk::Semaphore swapchain_acquire_semaphore;

    vk::Semaphore swapchain_release_semaphore;

    int32_t queue_index;
};

struct VulkanContext
{
    /// The Vulkan instance.
    vk::Instance instance;

    /// The Vulkan physical device.
    vk::PhysicalDevice gpu;

    /// The Vulkan device.
    vk::Device device;

    /// The Vulkan device queue.
    vk::Queue queue;

    /// The swapchain.
    vk::SwapchainKHR swapchain;

    /// The swapchain dimensions.
    SwapchainDimensions swapchain_dimensions;

    /// The surface we will render to.
    vk::SurfaceKHR surface;

    /// The queue family index where graphics work will be submitted.
    int32_t graphics_queue_index = -1;

    /// The image view for each swapchain image.
    std::vector<vk::ImageView> swapchain_image_views;

    /// The framebuffer for each swapchain image view.
    std::vector<vk::Framebuffer> swapchain_framebuffers;

    /// The renderpass description.
    vk::RenderPass render_pass;

    vk::CommandPool command_pool;

    /// The graphics pipeline.
    vk::Pipeline pipeline;

    vk::PipelineLayout pipeline_layout;

    /// The debug report callback.
    vk::DebugReportCallbackEXT debug_callback;

    /// A set of semaphores that can be reused.
    std::vector<vk::Semaphore> recycled_semaphores;

    /// A set of per-frame data.
    std::vector<PerFrame> per_frame;

    VmaAllocator memory_allocator{VK_NULL_HANDLE};

    vk::DescriptorPool descriptor_pool;

    vk::DescriptorSetLayout descriptor_set_layout;

    vk::Format depthFormat;
};

extern VulkanContext *context;

class VKBase
{
private:
    std::vector<vk::ExtensionProperties> device_extensions;
    Vent_Window window;

    void selectPhysicalDevice();

    void createInstance();

    void createDevice(const std::vector<const char *> &required_device_extensions);

    void createAllocator();

    void createCommandPool();

    void createDescriptorPool();

    void createDescriptorSetLayoutBinding();

    void createDepthFormat();

    [[nodiscard]] bool is_extension_supported(std::string const &requested_extension) const;

    vk::ShaderModule load_shader_module(const std::string_view &filename);

public:
    VKBase(const Vent_Window &window);

    void initVulkan();

    void shutdownVulkan();

    void init_per_frame(PerFrame &per_frame);

    void teardown_per_frame(PerFrame &per_frame);

    void createSwapchain();

    void createRenderPass();

    void createPipeline(const std::string_view &vertexShaderFilename, const std::string_view &fragmentShaderFilename);

    void destroyRenderpass();

    void destroySwapchain();

    void destroyPipeline();
};
