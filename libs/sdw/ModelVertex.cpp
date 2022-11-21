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


void ModelVertex::addNormal(glm::vec3 &normal, glm::vec3 &binormal, glm::vec3 &tangent)
{
    this->normal += normal;
    this->binormal += binormal;
    this->tangent += tangent;
}

void ModelVertex::normalize()
{
    normal = glm::normalize(normal);
    binormal = glm::normalize(binormal);
    tangent = glm::normalize(tangent);
}