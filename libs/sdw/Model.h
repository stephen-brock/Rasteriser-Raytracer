#pragma once
#include "ModelTriangle.h"
#include <vector>

class Model
{
    public:
        std::vector<ModelTriangle> triangles;
        Colour colour;
        Model();
        Model(std::vector<ModelTriangle> triangles, Colour colour);
};