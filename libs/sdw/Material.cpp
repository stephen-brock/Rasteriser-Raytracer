#include "Material.h"
#include <glm/glm.hpp>

Material::Material() = default;

Material::Material(glm::vec3 colour, float metallic)
{
    this->colour = colour;
    this->mirror = false;
    this->refract = false;
    this->metallic = metallic;
}
Material::Material(glm::vec3 colour, float metallic, bool mirror)
{
    this->colour = colour;
    this->mirror = mirror;
    this->refract = false;
    this->metallic = metallic;
}
Material::Material(glm::vec3 colour, float metallic, bool refract, float refractiveIndex)
{
    this->colour = colour;
    this->mirror = false;
    this->refract = refract;
    this->metallic = metallic;
    this->refractiveIndex = refractiveIndex;
}

void Material::transformNormal(glm::vec3 &normal, glm::vec3 &binormal, glm::vec3 &tangent, float u, float v) {}

glm::vec3 Material::sampleAlbedo(float u, float v)
{
    return this->colour;
}

Material::~Material() = default;