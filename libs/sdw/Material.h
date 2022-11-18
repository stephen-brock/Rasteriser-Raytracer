#pragma once
#include <glm/glm.hpp>

class Material
{
    public:
        glm::vec3 colour;
        virtual glm::vec3 sampleAlbedo(float u, float v);
        Material();
        Material(glm::vec3 colour);
        virtual ~Material();
};