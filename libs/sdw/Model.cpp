#include "Model.h"
#include "MatrixUtils.h"

Model::Model() 
{
    this->verts = new std::vector<ModelVertex>();
    this->transformedVerts = new std::vector<ModelVertex>();
    this->triangles = new std::vector<ModelTriangle>();
    this->material = nullptr;
    this->transform = glm::mat4(
        1.0f,0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        0.0f,0.0f,0.0f,1.0f
    );
}

Model::Model(Material* material)
{
    this->verts = new std::vector<ModelVertex>();
    this->transformedVerts = new std::vector<ModelVertex>();
    this->triangles = new std::vector<ModelTriangle>();
    this->material = material;
    this->transform = glm::mat4(
        1.0f,0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        0.0f,0.0f,0.0f,1.0f
    );
}

ModelVertex& Model::GetVertex(int index)
{
    return verts->at(index);
}

int Model::VertexAmount()
{
    return verts->size();
}

void Model::AddVertex(ModelVertex &vertex)
{
    verts->push_back(vertex);
}

void Model::AddTriangle(int v0, int v1, int v2)
{
	ModelVertex &x = verts->at(v0);
	ModelVertex &y = verts->at(v1);
	ModelVertex &z = verts->at(v2);
    glm::vec3 binormal = glm::normalize(y.pos - x.pos);
    glm::vec3 tangent = glm::normalize(z.pos - x.pos + glm::vec3(0,0.00001f,0));
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
        transformedVerts->push_back(v);
    }
    
}

void Model::TransformVerticies()
{
    for (int i = 0; i < verts->size(); i++)
    {
        transformedVerts->at(i).pos = glm::vec3(transform * glm::vec4(verts->at(i).pos, 1.0f));
        transformedVerts->at(i).normal = glm::vec3(transform * glm::vec4(verts->at(i).normal, 0.0f));
        transformedVerts->at(i).binormal = glm::vec3(transform * glm::vec4(verts->at(i).binormal, 0.0f));
        transformedVerts->at(i).tangent = glm::vec3(transform * glm::vec4(verts->at(i).tangent, 0.0f));
    }
}

Model::~Model()
{
    delete verts;
    delete triangles;
}