#include "Camera.hpp"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::handleInput(SDL_Keysym &event)
{
    switch (event.scancode)
    {
    case SDL_SCANCODE_W:
        camera_pos.z += 0.1F;
        break;
    case SDL_SCANCODE_S:
        camera_pos.z -= 0.1F;
        break;
    case SDL_SCANCODE_A:
        camera_pos.x += 0.1F;
        break;
    case SDL_SCANCODE_D:
        camera_pos.x -= 0.1F;
        break;

    default:
        break;
    }
}