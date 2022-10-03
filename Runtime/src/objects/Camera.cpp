#include "Camera.hpp"

static const float _SPEED = 10.0f;
static const float _SENSITIVITY_X = 0.165f;
static const float _SENSITIVITY_Y = 0.165f;

void Camera::handleInput(SDL_Keysym &event, float delta_time) noexcept
{
    const auto sin_pitch = glm::sin(rotation.x);
    const auto cos_pitch = glm::cos(rotation.x);
    switch (event.sym)
    {
    case SDLK_w:
        position.x += sin_pitch * _SPEED * delta_time;
        position.z += cos_pitch * _SPEED * delta_time;
        break;
    case SDLK_s:
        position.x -= sin_pitch * _SPEED * delta_time;
        position.z -= cos_pitch * _SPEED * delta_time;
        break;
    case SDLK_a:
        position.x += cos_pitch * _SPEED * delta_time;
        position.z -= sin_pitch * _SPEED * delta_time;
        break;
    case SDLK_d:
        position.x -= cos_pitch * _SPEED * delta_time;
        position.z += sin_pitch * _SPEED * delta_time;
        break;
        break;
    case SDLK_SPACE:
        position.y += _SPEED * delta_time;
        break;
    case SDLK_LSHIFT:
        position.y -= _SPEED * delta_time;
        break;
        // Debug
    case SDLK_q:
        rotation.x += 1;
        rotation.z += 1;

    default:
        break;
    }
}

void Camera::handleMouse(SDL_MouseMotionEvent &event) noexcept
{
    const auto deltaposition = glm::vec2(static_cast<float>(event.xrel), static_cast<float>(event.yrel));

    const auto moveposition = deltaposition * glm::vec2(_SENSITIVITY_X, _SENSITIVITY_Y) * 0.0073f;

    rotation.x -= moveposition.x;
    if (rotation.x > glm::two_pi<float>())
        rotation.x = 0.0f;
    else if (rotation.x < 0.0f)
        rotation.x = glm::two_pi<float>();

    rotation.y -= moveposition.y;
    if (rotation.y > glm::half_pi<float>() - 0.15f)
        rotation.y = glm::half_pi<float>() - 0.15f;
    else if (rotation.y < 0.15f - glm::half_pi<float>())
        rotation.y = 0.15f - glm::half_pi<float>();
}