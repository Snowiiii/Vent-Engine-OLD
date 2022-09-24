#pragma once

#include <glm/glm.hpp>
#include <SDL_keyboard.h>

class Camera
{
private:
public:
    glm::vec3 rotation = glm::vec3();
    glm::vec3 camera_pos = glm::vec3();

    Camera();
    ~Camera();

    void handleInput(SDL_Keysym &event);
};
