#include "Model.h"


Model::Model() = default;
Model::Model(std::vector<ModelTriangle> triangles, Colour* colour)
{
    this->triangles = triangles;
    this->colour = colour;
}