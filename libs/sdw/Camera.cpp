#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "MatrixUtils.h"
#include "Camera.h"
#include "Light.h"
#include <iostream>

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

glm::vec3 Camera::getRayDirection(float u, float v)
{
	float y = (v - (float)height / 2.0) / -focalLength;
	float x = (u - (float)width / 2.0) / focalLength;
	glm::vec4 localDir = glm::vec4(x, y, -1, 0);
	glm::vec4 dir = cameraToWorld * localDir;
	return glm::normalize(glm::vec3(dir));
}

RayTriangleIntersection Camera::getClosestIntersection(glm::vec3 origin, glm::vec3 rayDirection, std::vector<ModelTriangle> &triangles)
{
	RayTriangleIntersection closestIntersection = RayTriangleIntersection(glm::vec3(0,0,0), 10000000, ModelTriangle(), -1);

	for (int i = 0; i < triangles.size(); i++)
	{
		ModelTriangle triangle = triangles[i];
		glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
		glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
		glm::vec3 SPVector = origin - triangle.vertices[0];
		glm::mat3 DEMatrix(-rayDirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		bool hit = (possibleSolution.x > 0) && (possibleSolution.y >= 0.0) && (possibleSolution.y <= 1.0) && (possibleSolution.z >= 0.0) && (possibleSolution.z <= 1.0) && (possibleSolution.y + possibleSolution.z) <= 1.0;
		if (hit && possibleSolution.x < closestIntersection.distanceFromCamera && possibleSolution.x >= 0.025f)
		{
			float closestDistance = possibleSolution.x;
			glm::vec3 closestPoint = triangle.vertices[0] + e0 * possibleSolution.y + e1 * possibleSolution.z;
			closestIntersection = RayTriangleIntersection(closestPoint, closestDistance, triangle, i);
		}
	}

	return closestIntersection;
}

bool Camera::inShadow(RayTriangleIntersection &intersection, std::vector<ModelTriangle> &triangles, std::vector<Light> &lights)
{
	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 lightDir = lights[i].position - intersection.intersectionPoint; 
		RayTriangleIntersection shadowIntersection = Camera::getClosestIntersection(intersection.intersectionPoint, lightDir, triangles);
		if (shadowIntersection.triangleIndex != -1 && shadowIntersection.distanceFromCamera < 1)
		{
			return true;
		}
	}
	return false;
}

Colour Camera::renderTraced(int x, int y, std::vector<ModelTriangle> &triangles, std::vector<Light> &lights)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	glm::vec4 cameraPos = posFromMatrix(this->cameraToWorld);
	// std::cout << cameraPos.z << std::endl;
	RayTriangleIntersection intersection = Camera::getClosestIntersection(glm::vec3(cameraPos), rayDir, triangles);
	Colour col = intersection.intersectedTriangle.colour;
	if (intersection.triangleIndex != -1 && Camera::inShadow(intersection, triangles, lights))
	{
		col = Colour(0,0,0);
	}
	return col;
	// return Colour(fabs(rayDir.x) * 255, fabs(rayDir.y) * 255, fabs(rayDir.z) * 255);
}

void Camera::updateTransform()
{
	this->worldToCamera = glm::inverse(cameraToWorld);
}