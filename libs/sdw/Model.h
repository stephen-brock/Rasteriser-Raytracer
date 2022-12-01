#pragma once
#include "ModelTriangle.h"
#include <vector>
#include "ModelVertex.h"
#include "Material.h"
#include "TextureMap.h"
#include "BoundingBox.h"

class Model
{
    public:
        std::vector<ModelVertex> *transformedVerts;
        std::vector<ModelTriangle> *triangles;
        Material* material;
        glm::mat4 transform;
        BoundingBox boundingBox;
        Model();
        ~Model();
        Model(Material* material);
        void AddVertex(ModelVertex &vertex);
        int VertexAmount();
        ModelVertex& GetVertex(int index);
        void AddTriangle(int v0, int v1, int v2);
        void NormaliseNormals();
        void TransformVerticies();
        void Displace(TextureMap &amp, TextureMap &nrm, glm::vec2 offset, float scale);
    private:
        std::vector<ModelVertex> *verts;
};