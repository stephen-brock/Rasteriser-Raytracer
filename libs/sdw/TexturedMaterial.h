#pragma once
#include "Material.h"
#include <glm/glm.hpp>
#include "TextureMap.h"
#include "MatrixUtils.h"

class TexturedMaterial : public Material
{
    public:
        TextureMap *textureMap;
        TextureMap *normalMap;
        glm::vec3 sampleAlbedo(float u, float v) override;
        TexturedMaterial();
        TexturedMaterial(glm::vec3 colour, bool mirror, std::string &texpath, std::string &normalPath);
        TexturedMaterial(glm::vec3 colour, bool refract, float refractiveIndex, std::string &texpath, std::string &normalPath);
        void transformNormal(glm::vec3 &normal, glm::vec3 &binormal, glm::vec3 &tangent, float u, float v) override;
        ~TexturedMaterial();
        
    private:
        uint32_t sampleTexture(float u, float v);
};