#include <glm/glm.hpp>
#include "ModelVertex.h"

ModelVertex::ModelVertex() = default;
ModelVertex::ModelVertex(glm::vec3 position)
{
    this->position = position;
    this->normal = normal;
}
void ModelVertex::AddNormal(glm::vec3 normal)
{
    this->normal += normal;
}
void ModelVertex::Normalize()
{
    this->normal = glm::normalize(this->normal);
}
