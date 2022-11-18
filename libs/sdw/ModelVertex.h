#pragma once
#include <glm/glm.hpp>

struct ModelVertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    ModelVertex();
    ModelVertex(glm::vec3 pos);
};