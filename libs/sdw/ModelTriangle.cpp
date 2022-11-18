#include "ModelTriangle.h"
#include <utility>

ModelTriangle::ModelTriangle() = default;

ModelTriangle::ModelTriangle(const int &v0, const int &v1, const int &v2, Colour colour) :
		vertices({{v0, v1, v2}}), colour(std::move(colour)) {}

// ModelTriangle::ModelTriangle(const ModelTriangle& tri)
// {
// 	vertices = std::array<int, 3>(tri.vertices);
// 	colour = tri.colour;
// }

std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle) {
	os << "(" << triangle.vertices[0] << ", " << triangle.vertices[1] << ", " << triangle.vertices[2] << ")\n";
	return os;
}
