#include "ModelTriangle.h"
#include <utility>

ModelTriangle::ModelTriangle() = default;

ModelTriangle::ModelTriangle(const int &v0, const int &v1, const int &v2, Colour colour) :
		vertices({{v0, v1, v2}}), colour(std::move(colour)) {}

		
ModelTriangle::ModelTriangle(ModelTriangle &mt)
{
	this->vertices = std::array<int,3>();
	vertices[0] = mt.vertices[0];
	vertices[1] = mt.vertices[1];
	vertices[2] = mt.vertices[2];
	this->colour = mt.colour;
}

std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle) {
	os << "(" << triangle.vertices[0] << ", " << triangle.vertices[1] << ", " << triangle.vertices[2] << ")\n";
	return os;
}
