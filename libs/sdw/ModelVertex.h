#pragma once
#include <glm/glm.hpp>

struct ModelVertex
{
    glm::vec3 pos{};
    glm::vec3 normal{};
    glm::vec3 binormal{};
    glm::vec3 tangent{};
    glm::vec2 texcoord{};
    ModelVertex();
    ModelVertex(const ModelVertex &v);
    ModelVertex(glm::vec3 pos);
    void addNormal(glm::vec3 &normal, glm::vec3 &binormal, glm::vec3 &tangent);
    void normalize();
};