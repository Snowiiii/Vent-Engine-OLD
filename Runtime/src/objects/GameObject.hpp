#pragma once

#include <glm/glm.hpp>

class GameObject
{
private:
public:
    glm::vec3 position = glm::vec3();
    glm::vec3 rotation = glm::vec3();
    glm::vec3 scale = glm::vec3(1.0f);
};


