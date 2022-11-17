#include "Material.h"
#include <glm/glm.hpp>

Material::Material() = default;
Material::Material(glm::vec3 colour)
{
    this->colour = colour;
}

glm::vec3 Material::sampleAlbedo(float u, float v)
{
    return this->colour;
}

Material::~Material() = default;