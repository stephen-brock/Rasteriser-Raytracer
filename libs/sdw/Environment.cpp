#include "Environment.h"
#include "MatrixUtils.h"

Environment::Environment() = default;
Environment::Environment(std::string texPath, glm::vec3 colour)
{
    textureMap = new TextureMap(texPath);
    this->colour = colour;
}
glm::vec3 Environment::sampleEnvironment(glm::vec3 &rayDir)
{
    float y = (asin(-rayDir.y) + M_PI_2) / M_PI;
    float x = (atan(rayDir.z / rayDir.x) + M_PI_2) / M_PI;
    uint32_t sample = textureMap->sample(x, y);
    return intToVector(sample) * colour;
}

Environment::~Environment()
{
    delete textureMap;
}