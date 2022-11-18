#pragma once
#include <glm/glm.hpp>

struct ModelVertex
{
    glm::vec3 pos{};
    glm::vec3 normal{};
    glm::vec3 texcoord{};
    ModelVertex();
    ModelVertex(const ModelVertex &v);
    ModelVertex(glm::vec3 pos);
};