#include "ModelTriangle.h"
#include "ModelVertex.h"
#include <utility>

ModelTriangle::ModelTriangle() = default;

ModelTriangle::ModelTriangle(const ModelVertex &v0, const ModelVertex &v1, const ModelVertex &v2, Colour trigColour) :
		vertices({{v0, v1, v2}}), colour(std::move(trigColour)) {}

std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle) {
	os << "(" << triangle.vertices[0].position.x << ", " << triangle.vertices[0].position.y << ", " << triangle.vertices[0].position.z << ")\n";
	os << "(" << triangle.vertices[1].position.x << ", " << triangle.vertices[1].position.y << ", " << triangle.vertices[1].position.z << ")\n";
	os << "(" << triangle.vertices[2].position.x << ", " << triangle.vertices[2].position.y << ", " << triangle.vertices[2].position.z << ")\n";
	return os;
}
