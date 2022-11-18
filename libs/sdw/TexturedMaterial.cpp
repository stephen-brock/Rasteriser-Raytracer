#include "TexturedMaterial.h"
#include <glm/glm.hpp>
#include "MatrixUtils.h"

TexturedMaterial::TexturedMaterial() = default;
TexturedMaterial::TexturedMaterial(glm::vec3 colour, TextureMap texture)
{
    this->colour = colour;
    this->textureMap = texture;
}

glm::vec3 TexturedMaterial::intToVector(uint32_t colour) 
{
    //const uint32_t alphaMask = 0xFF000000;
    const uint32_t redMask = 0x00FF0000;
    const uint32_t greenMask = 0x0000FF00;
    const uint32_t blueMask = 0x000000FF;
    //int a = (colour & alphaMask) >> 24;
    int r = (colour & redMask) >> 16;
    int g = (colour & greenMask) >> 8;
    int b = (colour & blueMask);
	return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

uint32_t TexturedMaterial::sampleTexture(float u, float v)
{
	return this->textureMap.pixels[floor(fmod(v, 1) * this->textureMap.height) * this->textureMap.width + floor(fmod(u, 1) * this->textureMap.width)];
}

glm::vec3 TexturedMaterial::sampleAlbedo(float u, float v)
{
    return colour * intToVector(sampleTexture(u,v));
}

TexturedMaterial::~TexturedMaterial() = default;