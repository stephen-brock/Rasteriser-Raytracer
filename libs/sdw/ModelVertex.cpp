#include "ModelVertex.h"

ModelVertex::ModelVertex() = default;

ModelVertex::ModelVertex(const ModelVertex &v)
{
    pos = v.pos;
    normal = v.normal;
    texcoord = v.texcoord;
}
ModelVertex::ModelVertex(glm::vec3 pos)
{
    this->pos = pos;
}