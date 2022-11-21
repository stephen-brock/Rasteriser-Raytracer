#include "Model.h"
#include "MatrixUtils.h"

Model::Model() 
{
    this->verts = new std::vector<ModelVertex>();
    this->triangles = new std::vector<ModelTriangle>();
    this->material = nullptr;
}

Model::Model(Material* material)
{
    this->verts = new std::vector<ModelVertex>();
    this->triangles = new std::vector<ModelTriangle>();
    this->material = material;
}

void Model::AddTriangle(int v0, int v1, int v2)
{
	ModelVertex &x = verts->at(v0);
	ModelVertex &y = verts->at(v1);
	ModelVertex &z = verts->at(v2);
    glm::vec3 binormal = y.pos - x.pos;
    glm::vec3 tangent = z.pos - x.pos;
	glm::vec3 normal = glm::normalize(glm::cross(binormal, tangent));
	x.addNormal(normal, binormal, tangent);
	y.addNormal(normal, binormal, tangent);
	z.addNormal(normal, binormal, tangent);
	triangles->push_back(ModelTriangle(v0, v1, v2, vectorToColour(material->sampleAlbedo(0,0))));
}

void Model::NormaliseNormals()
{
    for (int i = 0; i < verts->size(); i++)
    {
        ModelVertex& v = (verts->at(i));
        v.normalize();
    }
    
}

Model::~Model()
{
    delete verts;
    delete triangles;
}