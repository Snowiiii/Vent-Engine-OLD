#pragma once

#include <glm/glm.hpp>
#include <SDL_keyboard.h>

#include "GameObject.hpp"

class Camera : public GameObject
{
private:
public:
    void handleInput(SDL_Keysym &event);
};
