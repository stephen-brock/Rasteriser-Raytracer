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

RayTriangleIntersection Camera::getClosestIntersection(glm::vec3 origin, glm::vec3 rayDirection, std::vector<Model> &models, int ignoreIndex)
{
	RayTriangleIntersection closestIntersection = RayTriangleIntersection(glm::vec3(0,0,0), 1000, nullptr, -1);

	for (int m = 0; m < models.size(); m++)
	{
		Model* model = &models[m];
		for (int i = 0; i < model->triangles.size(); i++)
		{
			if (i == ignoreIndex)
			{
				continue;
			}
			ModelTriangle triangle = model->triangles[i];
			glm::vec3 e0 = triangle.vertices[1].position - triangle.vertices[0].position;
			glm::vec3 e1 = triangle.vertices[2].position - triangle.vertices[0].position;
			glm::vec3 SPVector = origin - triangle.vertices[0].position;
			glm::mat3 DEMatrix(-rayDirection, e0, e1);
			glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
			bool hit = (possibleSolution.x > 0) && (possibleSolution.y >= 0.0) && (possibleSolution.y <= 1.0) && (possibleSolution.z >= 0.0) && (possibleSolution.z <= 1.0) && (possibleSolution.y + possibleSolution.z) <= 1.0;
			if (hit && possibleSolution.x < closestIntersection.distanceFromCamera && possibleSolution.x >= 0.001f)
			{
				float closestDistance = possibleSolution.x;
				glm::vec3 closestPoint = triangle.vertices[0].position + e0 * possibleSolution.y + e1 * possibleSolution.z;
				closestIntersection = RayTriangleIntersection(closestPoint, closestDistance, model, i);
				closestIntersection.u = possibleSolution.y;
				closestIntersection.v = possibleSolution.z;
			}
		}
	}
	

	return closestIntersection;
}

bool Camera::inShadow(RayTriangleIntersection &intersection, std::vector<Model> &models, glm::vec3 lightDir)
{
	RayTriangleIntersection shadowIntersection = Camera::getClosestIntersection(intersection.intersectionPoint, lightDir, models, intersection.triangleIndex);
	if (shadowIntersection.triangleIndex != -1 && shadowIntersection.distanceFromCamera < 1)
	{
		return true;
	}
	return false;
}

Colour Camera::renderTraced(int x, int y, std::vector<Model> &models, std::vector<Light> &lights)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	glm::vec4 cameraPos = posFromMatrix(this->cameraToWorld);
	RayTriangleIntersection intersection = Camera::getClosestIntersection(glm::vec3(cameraPos), rayDir, models);
	if (intersection.triangleIndex == -1)
	{
		return Colour(0,0,0);
	}
	auto tri = intersection.intersectedModel->triangles[intersection.triangleIndex];
	glm::vec3 vecCol = colourToVector(tri.colour);
	glm::vec3 diffuseColour = glm::vec3(0,0,0);
	glm::vec3 specularColour = glm::vec3(0,0,0);
	glm::vec3 ambientColour = glm::vec3(0.1f, 0.1f, 0.2f);

	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 lightDir = lights[i].position - intersection.intersectionPoint; 
		if (!Camera::inShadow(intersection, models, lightDir))
		{
			float lightLength = glm::length(lightDir);
			lightDir /= lightLength == 0 ? 1 : lightLength;
			glm::vec3 lightIntensity = lights[i].colour / (float)(2 * M_PI * lightLength * lightLength);
			auto verts = tri.vertices;
			float u = intersection.u;
			float v = intersection.v;
			float w = 1 - intersection.u - intersection.v;
			glm::vec3 normal = glm::normalize(verts[0].normal * w + verts[1].normal * u + verts[2].normal * v);
			float ldn = glm::dot(lightDir, normal);
			ldn = ldn < 0 ? 0 : ldn;
			diffuseColour += ldn * lightIntensity;
			float specDot = glm::dot(glm::reflect(rayDir, normal), lightDir);
			specDot = specDot < 0 ? 0 : specDot;
			specularColour += powf(specDot, 32) * lightIntensity;
		}
	}
	vecCol *= (diffuseColour + ambientColour);
	vecCol += specularColour;
	return vectorToColour(vecCol);
}

void Camera::updateTransform()
{
	this->worldToCamera = glm::inverse(cameraToWorld);
}