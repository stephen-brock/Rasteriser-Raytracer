#pragma once
#include "Material.h"
#include <glm/glm.hpp>
#include "TextureMap.h"
#include "MatrixUtils.h"

class TexturedMaterial : public Material
{
    public:
        TextureMap *textureMap;
        glm::vec3 sampleAlbedo(float u, float v);
        TexturedMaterial();
        TexturedMaterial(glm::vec3 colour, bool mirror, bool refract, std::string &texpath);
        ~TexturedMaterial();
        
    private:
        uint32_t sampleTexture(float u, float v);
};