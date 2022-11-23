#include "Light.h"
#include <glm/glm.hpp>
#include "Colour.h"


Light::Light()
{
    this->colour = glm::vec3(1,1,1);
    this->position = glm::vec3(0,0,0);
}

Light::Light(glm::vec3 position, glm::vec3 colour)
{
    this->colour = colour;
    this->position = position;
}

float Light::lightAttenuation(glm::vec3 lightDir)
{
	return 1 / fmax(1, (4.0f * (float)M_PI * glm::dot(lightDir, lightDir)));
}