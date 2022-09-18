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
		
		while (!renderer.window.shouldClose())
		{
			float currentTime = glfwGetTime();
			float delta = currentTime - previousTime;
			previousTime = currentTime;

			renderer.window.update();
			renderer.update(delta);

		//	spdlog::debug("FPS {}, {} ms", 1000 / delta, delta);
		}
	}
	catch (const std::exception &e)
	{
		spdlog::critical(e.what());
		return -1;
	}

	return 0;
}
