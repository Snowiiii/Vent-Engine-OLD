#include "Vent_Window.hpp"

Vent_Window::Vent_Window(const uint32_t &width,const uint32_t &height, const std::string_view &title) : width(width), height(height)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Error initializing SDL: %s", SDL_GetError());
		exit(1);
	}
	handle = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
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

bool Vent_Window::handleEvents(Camera &camera)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			return false;
			break;
		case SDL_KEYDOWN:
			camera.handleInput(event.key.keysym);
			break;
		}
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