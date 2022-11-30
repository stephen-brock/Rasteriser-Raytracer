#pragma once
#include <glm/glm.hpp>

class Material
{
    public:
        glm::vec3 colour;
        bool mirror;
        bool refract;
        float refractiveIndex;
        float metallic;
        virtual glm::vec3 sampleAlbedo(float u, float v);
        Material();
        Material(glm::vec3 colour, float metallic);
        Material(glm::vec3 colour, float metallic, bool mirror);
        Material(glm::vec3 colour, float metallic, bool refract, float refractiveIndex);
        virtual void transformNormal(glm::vec3 &normal, glm::vec3 &binormal, glm::vec3 &tangent, float u, float v);
        virtual ~Material();
};