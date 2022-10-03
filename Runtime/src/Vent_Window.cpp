#include "Vent_Window.hpp"

Vent_Window::Vent_Window(const uint32_t &width, const uint32_t &height, const std::string_view &title) : width(width), height(height)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Error initializing SDL: %s", SDL_GetError());
		exit(1);
	}
	handle = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (!handle)
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Error creating SDL window");
		exit(1);
	}
}

Vent_Window::~Vent_Window()
{
	if (handle)
	{
		SDL_DestroyWindow(handle);
	}
	SDL_Quit();
}

void Vent_Window::grabMouse(const bool grab)
{
	this->_mouseGrabbed = grab;
	SDL_ShowCursor(grab ? SDL_DISABLE : SDL_ENABLE);
	SDL_SetWindowGrab(handle, grab ? SDL_TRUE : SDL_FALSE);
	SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
}

bool Vent_Window::handleEvents(SDL_Event &event, Camera &camera)
{
	switch (event.type)
	{
	case SDL_QUIT:
		return false;
		break;
	case SDL_KEYDOWN:
		camera.handleInput(event.key.keysym, 003.0F);
		break;
	case SDL_MOUSEMOTION:
		camera.handleMouse(event.motion);
		break;
	}
	return true;
}

vk::SurfaceKHR Vent_Window::createSurface(vk::Instance &instance)
{
	VkSurfaceKHR _surface;
	if (!SDL_Vulkan_CreateSurface(handle, static_cast<VkInstance>(instance), &_surface))
		throw std::runtime_error("Failed to create window surface!");
	return static_cast<vk::SurfaceKHR>(_surface);
}