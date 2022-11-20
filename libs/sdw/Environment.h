#pragma once
#include "TextureMap.h"
#include <glm/glm.hpp>

class Environment
{
    public:
        Environment();
        Environment(std::string texPath);
        glm::vec3 sampleEnvironment(glm::vec3 &rayDir);
        ~Environment();
    private:
     TextureMap* textureMap;
};