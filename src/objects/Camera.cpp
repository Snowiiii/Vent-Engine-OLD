#include "Camera.hpp"

GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}

void Camera::handleInput(SDL_Keysym &event)
{
    switch (event.sym)
    {
    case SDLK_w:
        pos.z += 0.1F;
        break;
    case SDLK_s:
        pos.z -= 0.1F;
        break;
    case SDLK_a:
        pos.x += 0.1F;
        break;
    case SDLK_d:
        pos.x -= 0.1F;
        break;
    // Debug
    case SDLK_q:
        rotation.x += 1;
        rotation.z += 1;
        break;

    default:
        break;
    }
}