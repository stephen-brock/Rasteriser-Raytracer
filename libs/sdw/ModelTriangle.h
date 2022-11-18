#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"

struct ModelTriangle {
	std::array<int, 3> vertices{};
	Colour colour{};

	ModelTriangle();
	// ModelTriangle(const ModelTriangle& tri);
	ModelTriangle(const int &v0, const int &v1, const int &v2, Colour trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
