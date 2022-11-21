#pragma once
#include <glm/glm.hpp>

class Material
{
    public:
        glm::vec3 colour;
        bool mirror;
        bool refract;
        float refractiveIndex;
        virtual glm::vec3 sampleAlbedo(float u, float v);
        Material();
        Material(glm::vec3 colour);
        Material(glm::vec3 colour, bool mirror);
        Material(glm::vec3 colour, bool refract, float refractiveIndex);
        virtual ~Material();
};