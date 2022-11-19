#pragma once
#include "ModelTriangle.h"
#include <vector>
#include "ModelVertex.h"
#include "Material.h"

class Model
{
    public:
        std::vector<ModelVertex> *verts;
        std::vector<ModelTriangle> *triangles;
        Material* material;
        Model();
        ~Model();
        Model(Material* material);
        void AddTriangle(int v0, int v1, int v2);
        void NormaliseNormals();
};