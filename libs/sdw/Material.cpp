#include "Material.h"
#include <glm/glm.hpp>

Material::Material() = default;
Material::Material(glm::vec3 colour, bool mirror, bool refract)
{
    this->colour = colour;
    this->mirror = mirror;
    this->refract = refract;
}

glm::vec3 Material::sampleAlbedo(float u, float v)
{
    return this->colour;
}

Material::~Material() = default;