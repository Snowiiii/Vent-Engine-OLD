#pragma once

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL.h>

#include "GameObject.hpp"

class Camera : public GameObject
{
private:
public:

   virtual void handleInput(SDL_Keysym &event, float delta_time) noexcept;

   virtual void handleMouse(SDL_MouseMotionEvent &event) noexcept;
};
