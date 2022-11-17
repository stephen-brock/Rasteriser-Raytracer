#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"
#include "Model.h"

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint;
	float distanceFromCamera;
	Model* intersectedModel;
	size_t triangleIndex;
	float u,v;

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, Model* model, size_t index);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
