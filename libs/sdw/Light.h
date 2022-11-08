#pragma once

#include "Colour.h"
#include <glm/glm.hpp>

class Light
{
    public:
        glm::vec3 position;
        glm::vec3 colour;
        Light();
        Light(glm::vec3 position, glm::vec3 colour);
};