#include "Vent-Editor.hpp"

int main(int argc, char const *argv[])
{
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    try
    {
        renderer = std::make_unique<Renderer>();

        Vent_GUI gui{renderer->window};

        float delta = 0.0f;
        uint64_t perfCounterFrequency = SDL_GetPerformanceFrequency();
        uint64_t lastCounter = SDL_GetPerformanceCounter();

        SDL_Event event;
        bool running = true;
        while (running)
        {
            while (SDL_PollEvent(&event))
            {
                gui.handleSDLEvent(event);
                running = renderer->window.handleEvents(event, renderer->camera);
            }

            uint32_t index = renderer->onPreUpdate(delta);
            vk::CommandBuffer cmd = renderer->onPreDraw(index);
            gui.update(cmd);
            renderer->onPostDraw(cmd, index);
            renderer->onPostUpdate(index);

            uint64_t endCounter = SDL_GetPerformanceCounter();
            uint64_t counterElapsed = endCounter - lastCounter;
            delta = (static_cast<float>(counterElapsed)) / static_cast<float>(perfCounterFrequency);
            lastCounter = endCounter;

            //	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "FPS %f, %f ms", 1000 / delta, delta);
        }
    }
    catch (const std::exception &e)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error in Vent-Runtime: %s", e.what());
        if (renderer)
        {
            renderer.reset();
        }

        return -1;
    }

    return 0;
}