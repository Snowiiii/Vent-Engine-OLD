#include "Vent_Window.hpp"

Vent_Window::Vent_Window(uint32_t width, uint32_t height, const char *title) : width(width), height(height)
{
	this->createWindow(title);
}

Vent_Window::~Vent_Window()
{
	glfwTerminate();
}

void error_callback(int error, const char *description)
{
	spdlog::error("GLFW Error (code {}): {}", error, description);
}

void Vent_Window::createWindow(const char *title)
{
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, false);
#endif

	if (!glfwInit())
	{
		spdlog::critical("Failed to Initialize GLFW");
		exit(-1);
	}

	glfwSetErrorCallback(error_callback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	handle = glfwCreateWindow(width, height, title, nullptr, nullptr);

	if (!handle)
	{
		glfwTerminate();
		spdlog::critical("Failed to create GLFW Window");
		exit(-1);
	}

	glfwSetWindowUserPointer(handle, this);
	glfwSetFramebufferSizeCallback(handle, framebufferResizeCallback);
}

void Vent_Window::update() const noexcept
{
	glfwPollEvents();
}
void Vent_Window::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	auto vWindow = reinterpret_cast<Vent_Window *>(glfwGetWindowUserPointer(window));
	vWindow->width = width;
	vWindow->height = height;
}

vk::SurfaceKHR Vent_Window::createSurface(vk::Instance instance)
{
	VkSurfaceKHR _surface;
	VkResult err = glfwCreateWindowSurface(static_cast<VkInstance>(instance), handle, nullptr, &_surface);
	if (err != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
	return static_cast<vk::SurfaceKHR>(_surface);
}
