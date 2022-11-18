#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint;
	float distanceFromCamera;
	size_t triangleIndex;
	float u;
	float v;

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, size_t index);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
