#pragma once

#include <glm/glm.hpp>

struct ModelVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    ModelVertex();
    ModelVertex(glm::vec3 position);
    void AddNormal(glm::vec3 normal);
    void Normalize();
};