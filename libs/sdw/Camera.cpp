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

RayTriangleIntersection Camera::getClosestIntersection(glm::vec3 &origin, glm::vec3 &rayDirection, std::vector<ModelTriangle> &triangles)
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

bool Camera::inShadow(RayTriangleIntersection &intersection, std::vector<ModelTriangle> &triangles, glm::vec3 &lightDir)
{
	RayTriangleIntersection shadowIntersection = Camera::getClosestIntersection(intersection.intersectionPoint, lightDir, triangles);
	if (shadowIntersection.triangleIndex != -1 && shadowIntersection.distanceFromCamera < 1)
	{
		return true;
	}
	return false;
}

Colour Camera::renderTraced(int x, int y, std::vector<ModelTriangle> &triangles, std::vector<Light> &lights)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	glm::vec3 cameraPos = glm::vec3(posFromMatrix(this->cameraToWorld));
	RayTriangleIntersection intersection = Camera::getClosestIntersection(cameraPos, rayDir, triangles);

	glm::vec3 normal = intersection.intersectedTriangle.normal;

	glm::vec3 albedo = colourToVector(intersection.intersectedTriangle.colour);
	glm::vec3 lightIntensity = glm::vec3(0,0,0);
	glm::vec3 specularIntensity = glm::vec3(0,0,0);
	glm::vec3 ambientIntensity = glm::vec3(0.2f,0.3f,0.35f);
	
	if (intersection.triangleIndex == -1)
	{
		return Colour(0,0,0);
	}

	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 lightDir = lights[i].position - intersection.intersectionPoint; 
		if (!Camera::inShadow(intersection, triangles, lightDir))
		{
			glm::vec3 lightCol = lights[i].colour / (2.0f * (float)M_PI * glm::dot(lightDir, lightDir));
			lightDir = glm::normalize(lightDir);
			float ldn = glm::dot(lightDir, normal);
			ldn = ldn < 0 ? 0 : ldn;
			lightIntensity += lightCol * ldn;
			glm::vec3 refl = glm::reflect(rayDir, normal);
			float rdl = glm::dot(lightDir, refl);
			rdl = rdl < 0 ? 0 : rdl;
			specularIntensity += lightCol * powf(rdl, 32);
		}
	}

	lightIntensity += ambientIntensity;

	glm::vec3 finalColour = albedo * lightIntensity + specularIntensity;
	
	return vectorToColour(finalColour);
}

void Camera::updateTransform()
{
	this->worldToCamera = glm::inverse(cameraToWorld);
}