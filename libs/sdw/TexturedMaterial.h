#pragma once
#include "Material.h"
#include <glm/glm.hpp>
#include "TextureMap.h"

class TexturedMaterial : public Material
{
    public:
        TextureMap textureMap;
        glm::vec3 sampleAlbedo(float u, float v);
        TexturedMaterial();
        TexturedMaterial(glm::vec3 colour, TextureMap texture);
        ~TexturedMaterial();
        
    private:
        uint32_t sampleTexture(float u, float v);
        glm::vec3 intToVector(uint32_t colour); 
};