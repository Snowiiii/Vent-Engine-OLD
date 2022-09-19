#pragma once

#include "../Vent_Window.hpp"
#include <vk_mem_alloc.h>
#include <volk.h>

#include <glm/glm.hpp>

#include <cassert>
#include <vector>

#ifndef NDEBUG
#define VALIDATION_LAYERS
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions();
    static vk::VertexInputBindingDescription getBindingDescription();

    bool operator==(const Vertex &other) const
    {
        return pos == other.pos;
    }
};

struct Texture
{
    vk::Sampler sampler;

    vk::Image image;

    vk::ImageLayout image_layout;

    vk::DeviceMemory device_memory;

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

    void createDescriptorPool();

    void createDescriptorSetLayoutBinding();

    bool is_extension_supported(std::string const &requested_extension) const;

    vk::ShaderModule load_shader_module(const char *filename);

public:
    VKBase(const Vent_Window &window);

    void initVulkan();

    void shutdownVulkan();

    void init_per_frame(PerFrame &per_frame);

    void teardown_per_frame(PerFrame &per_frame);

    void createSwapchain();

    void createRenderPass();

    void createPipeline(const char *vertexShaderFilename, const char *fragmentShaderFilename);

    void destroyRenderpass();

    void destroySwapchain();

    void destroyPipeline();
};
