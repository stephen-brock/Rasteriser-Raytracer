#include "ModelVertex.h"

ModelVertex::ModelVertex() = default;
ModelVertex::ModelVertex(glm::vec3 pos)
{
    this->pos = pos;
}