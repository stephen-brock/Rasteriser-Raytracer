#include "TexturedMaterial.h"
#include <glm/glm.hpp>
#include "MatrixUtils.h"

TexturedMaterial::TexturedMaterial() = default;

TexturedMaterial::TexturedMaterial(glm::vec3 colour, bool mirror, std::string &texpath, std::string &normalPath)
{
    this->colour = colour;
    this->mirror = mirror;
    this->refract = false;
    this->textureMap = new TextureMap(texpath);
    this->normalMap = new TextureMap(normalPath);
}
TexturedMaterial::TexturedMaterial(glm::vec3 colour, bool refract, float refractiveIndex, std::string &texpath, std::string &normalPath)
{
    this->colour = colour;
    this->mirror = mirror;
    this->refract = refract;
    this->textureMap = new TextureMap(texpath);
    this->refractiveIndex = refractiveIndex;
    this->normalMap = new TextureMap(normalPath);
}

glm::vec3 TexturedMaterial::sampleAlbedo(float u, float v)
{
    uint32_t col = textureMap->sample(u, v);
    return colour * intToVector(col);
}

void TexturedMaterial::transformNormal(glm::vec3 &normal, glm::vec3 &binormal, glm::vec3 &tangent, float u, float v)
{
    glm::vec3 absOffset = intToVector(normalMap->sample(u,v));
    glm::vec3 offset = (absOffset - glm::vec3(0.5f, 0.5f, 0.5f)) * 2.0f;
    normal = (normal * offset.y + binormal * offset.x + tangent * offset.z);
}

TexturedMaterial::~TexturedMaterial()
{
    delete textureMap;
    delete normalMap;
}