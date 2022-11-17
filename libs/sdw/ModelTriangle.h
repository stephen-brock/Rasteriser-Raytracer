#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"
#include "ModelVertex.h"

struct ModelTriangle {
	std::array<ModelVertex, 3> vertices{};
	Colour colour{};

	ModelTriangle();
	ModelTriangle(const ModelVertex &v0, const ModelVertex &v1, const ModelVertex &v2, Colour trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
