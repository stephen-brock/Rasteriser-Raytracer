#pragma once
#include <glm/glm.hpp>

struct BoundingBox
{
    glm::vec3 from;
    glm::vec3 to;
    BoundingBox();
    BoundingBox(glm::vec3 from, glm::vec3 to);
    bool Hit(glm::vec3 &origin, glm::vec3 &dir);
};