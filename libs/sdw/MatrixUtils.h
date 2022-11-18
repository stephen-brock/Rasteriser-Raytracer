#pragma once
#include <glm/glm.hpp>
#include "Colour.h"


glm::vec4 posFromMatrix(glm::mat4 mat);

glm::mat4 xRotation(glm::mat4 mat, float angle);

glm::mat4 yRotation(glm::mat4 mat, float angle);

glm::mat4 zRotation(glm::mat4 mat, float angle);

glm::mat4 move(glm::mat4 mat, glm::vec3 add);

glm::mat4 matrixTRS(glm::vec3 pos, glm::vec3 eulerAngles);

glm::mat4 lookAt(glm::mat4 mat, glm::vec3 origin);

Colour vectorToColour(glm::vec3 col);

glm::vec3 colourToVector(Colour col);