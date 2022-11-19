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
	glm::vec3 normal = glm::normalize(glm::cross(y.pos - x.pos, z.pos - x.pos));
	
	x.normal += normal;
	y.normal += normal;
	z.normal += normal;
	triangles->push_back(ModelTriangle(v0, v1, v2, vectorToColour(material->sampleAlbedo(0,0))));
}

Model::~Model()
{
    delete verts;
    delete triangles;
}