#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "MatrixUtils.h"
#include "Camera.h"

Camera::Camera()
{
	width = 0;
	height = 0;
}

Camera::Camera(float focalLength, glm::mat4 cameraToWorld, int width, int height)
{
	this->focalLength = focalLength;
	this->cameraToWorld = cameraToWorld;
	this->width = width;
	this->height = height;
}

glm::vec3 Camera::getCanvasIntersectionPoint(glm::vec4 vertexPosition)
{
	glm::vec4 cPos = vertexPosition * worldToCamera;
	cPos *= 230.0f;
	cPos.z = -cPos.z;
	float u = focalLength * cPos.x / fabs(cPos.z) + width / 2;
	float v = height / 2 - focalLength * cPos.y / fabs(cPos.z);

	return glm::vec3(u, v, cPos.z);
}

glm::vec3 Camera::getRayDirection(int x, int y)
{
	glm::vec4 localDir = glm::normalize(glm::vec4(
		2.0 * ((float)x / width) - 1.0,
		2.0 * ((float)(height - y) / height) - 1.0,
		-this->focalLength, 0));
	localDir.x *= (float)width / height;
	glm::vec4 rayDir = this->cameraToWorld * localDir;
	return glm::vec3(rayDir.x, rayDir.y, rayDir.z);
}

RayTriangleIntersection Camera::getClosestIntersection(glm::vec3 rayDirection, std::vector<ModelTriangle> &triangles)
{
	glm::vec4 origin = posFromMatrix(this->cameraToWorld);
	glm::vec3 cameraOrigin = glm::vec3(origin.x, origin.y, origin.z);
	RayTriangleIntersection closestIntersection = RayTriangleIntersection(glm::vec3(0,0,0), 10000000, ModelTriangle(), 0);

	for (int i = 0; i < triangles.size(); i++)
	{
		ModelTriangle triangle = triangles[i];
		glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
		glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
		glm::vec3 SPVector = cameraOrigin - triangle.vertices[0];
		glm::mat3 DEMatrix(-rayDirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		bool hit = (possibleSolution.x >= 0) && (possibleSolution.y >= 0.0) && (possibleSolution.y <= 1.0) && (possibleSolution.z >= 0.0) && (possibleSolution.z <= 1.0) && (possibleSolution.y + possibleSolution.z) <= 1.0;
		if (hit && possibleSolution.x < closestIntersection.distanceFromCamera)
		{
			float closestDistance = possibleSolution.x;
			glm::vec3 closestPoint = triangle.vertices[0] + e0 * possibleSolution.y + e1 * possibleSolution.z;
			closestIntersection = RayTriangleIntersection(closestPoint, closestDistance, triangle, -1);
		}
	}

	return closestIntersection;
}

Colour Camera::renderTraced(int x, int y, std::vector<ModelTriangle> &triangles)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	RayTriangleIntersection intersection = this->getClosestIntersection(rayDir, triangles);
	return intersection.intersectedTriangle.colour;
	//return Colour(fabs(rayDir.x) * 255, fabs(rayDir.y) * 255, fabs(rayDir.z) * 255);
}

void Camera::updateTransform()
{
	this->worldToCamera = glm::inverse(cameraToWorld);
}