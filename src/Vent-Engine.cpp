#include "Vent-Engine.hpp"

int main(int argc, char *argv[])
{
#ifndef NDEBUG
	spdlog::warn("Application running in Debug Mode");
	spdlog::set_level(spdlog::level::debug);
#endif

	try
	{
		Renderer renderer{};

		float delta = 0.0f;
		uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
		uint64_t lastCounter = SDL_GetPerformanceCounter();

		while (renderer.window.handleEvents())
		{
			renderer.update(delta);
			uint64_t endCounter = SDL_GetPerformanceCounter();
			uint64_t counterElapsed = endCounter - lastCounter;
			delta = ((float)counterElapsed) / (float)perfCounterFrequency;
			lastCounter = endCounter;

			spdlog::debug("FPS {}, {} ms", 1000 / delta, delta);
		}
	}
	catch (const std::exception &e)
	{
		spdlog::critical(e.what());
		return -1;
	}

	return 0;
}
