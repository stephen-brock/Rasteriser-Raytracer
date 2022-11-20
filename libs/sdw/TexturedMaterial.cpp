#include "TexturedMaterial.h"
#include <glm/glm.hpp>
#include "MatrixUtils.h"

TexturedMaterial::TexturedMaterial() = default;
TexturedMaterial::TexturedMaterial(glm::vec3 colour, bool mirror, std::string &texpath)
{
    this->colour = colour;
    this->mirror = mirror;
    this->textureMap = new TextureMap(texpath);
}
glm::vec3 TexturedMaterial::sampleAlbedo(float u, float v)
{
    uint32_t col = textureMap->sample(u, v);
    return colour * intToVector(col);
}

TexturedMaterial::~TexturedMaterial()
{
    delete textureMap;
}