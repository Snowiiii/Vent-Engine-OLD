#include "Vent_Window.hpp"

Vent_Window::Vent_Window(uint32_t width, uint32_t height, const char *title) : width(width), height(height)
{
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		spdlog::critical("Error initializing SDL: {}", SDL_GetError());
		exit(1);
	}

	handle = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	if (!handle)
	{
		spdlog::critical("Error creating SDL window");
		exit(1);
	}
}

Vent_Window::~Vent_Window()
{
	SDL_DestroyWindow(handle);
	SDL_Quit();
}

bool Vent_Window::handleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			return false;
		}
	}
	return true;
}

vk::SurfaceKHR Vent_Window::createSurface(vk::Instance instance)
{
	VkSurfaceKHR _surface;
	if (!SDL_Vulkan_CreateSurface(handle, static_cast<VkInstance>(instance), &_surface))
		throw std::runtime_error("Failed to create window surface!");
	return static_cast<vk::SurfaceKHR>(_surface);
}