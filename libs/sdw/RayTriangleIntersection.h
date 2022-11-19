#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint;
	float distanceFromCamera;
	size_t triangleIndex;
	size_t modelIndex;
	float u;
	float v;

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, size_t index, size_t modelIndex);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
