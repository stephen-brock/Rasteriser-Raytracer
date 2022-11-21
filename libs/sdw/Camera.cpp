#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "MatrixUtils.h"
#include "Camera.h"
#include "Light.h"
#include <iostream>

const int MaxRayDepth = 4;

Camera::Camera()
{
	width = 0;
	height = 0;
}

Camera::Camera(float focalLength, glm::mat4 cameraToWorld, int width, int height, Environment* environment)
{
	this->focalLength = focalLength;
	this->cameraToWorld = cameraToWorld;
	this->width = width;
	this->height = height;
	this->environment = environment;
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
	glm::vec4 dir = localDir * cameraToWorld;
	return glm::normalize(glm::vec3(dir));
}

RayTriangleIntersection Camera::getClosestIntersection(glm::vec3 &origin, glm::vec3 &rayDirection, std::vector<Model*> &models)
{
	RayTriangleIntersection closestIntersection = RayTriangleIntersection(glm::vec3(0,0,0), 10000000, -1, -1);

	for (int m = 0; m < models.size(); m++)
	{
		Model &model = *models[m];

		for (int i = 0; i < model.triangles->size(); i++)
		{
			ModelTriangle triangle = model.triangles->at(i);
			glm::vec3 v0 = model.verts->at(triangle.vertices[0]).pos;
			glm::vec3 v1 = model.verts->at(triangle.vertices[1]).pos;
			glm::vec3 v2 = model.verts->at(triangle.vertices[2]).pos;
			glm::vec3 e0 = v1 - v0;
			glm::vec3 e1 = v2 - v0;
			glm::vec3 SPVector = origin - v0;
			glm::mat3 DEMatrix(-rayDirection, e0, e1);
			glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
			bool hit = (possibleSolution.x > 0) && (possibleSolution.y >= 0.0) && (possibleSolution.y <= 1.0) && (possibleSolution.z >= 0.0) && (possibleSolution.z <= 1.0) && (possibleSolution.y + possibleSolution.z) <= 1.0;
			if (hit && possibleSolution.x < closestIntersection.distanceFromCamera && possibleSolution.x >= 0.025f)
			{
				float closestDistance = possibleSolution.x;
				glm::vec3 closestPoint = v0 + e0 * possibleSolution.y + e1 * possibleSolution.z;
				closestIntersection = RayTriangleIntersection(closestPoint, closestDistance, i, m);
				closestIntersection.u = possibleSolution.y;
				closestIntersection.v = possibleSolution.z;
			}
		}
	}
	

	return closestIntersection;
}

bool Camera::inShadow(RayTriangleIntersection &intersection, std::vector<Model*> &models, glm::vec3 &lightDir)
{
	RayTriangleIntersection shadowIntersection = Camera::getClosestIntersection(intersection.intersectionPoint, lightDir, models);
	if (shadowIntersection.triangleIndex != -1 && shadowIntersection.distanceFromCamera < 1)
	{
		return true;
	}
	return false;
}

glm::vec3 Camera::render(glm::vec3 &albedo, glm::vec3 &normal, RayTriangleIntersection &intersection, glm::vec3 &rayDir, std::vector<Model*> &models, std::vector<Light> &lights)
{
	glm::vec3 lightIntensity = glm::vec3(0,0,0);
	glm::vec3 specularIntensity = glm::vec3(0,0,0);
	glm::vec3 ambientIntensity = glm::vec3(0.9f,1.0f,1.5f);
	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 lightDir = lights[i].position - intersection.intersectionPoint; 
		if (!Camera::inShadow(intersection, models, lightDir))
		{
			glm::vec3 lightCol = lights[i].colour / (4.0f * (float)M_PI * glm::dot(lightDir, lightDir));
			lightDir = glm::normalize(lightDir);
			float ldn = glm::dot(lightDir, normal);
			ldn = ldn < 0 ? 0 : ldn;
			lightIntensity += lightCol * ldn;
			glm::vec3 refl = glm::reflect(rayDir, normal);
			float rdl = glm::dot(lightDir, refl);
			rdl = rdl < 0 ? 0 : rdl;
			specularIntensity += lightCol * powf(rdl, 128);
		}
	}

	lightIntensity += ambientIntensity;
	glm::vec3 finalColour = lightIntensity * albedo + specularIntensity;
	return finalColour;
}

glm::vec3 Camera::renderRay(glm::vec3 &origin, glm::vec3 &rayDir, std::vector<Model*> &models, std::vector<Light> &lights, int currentDepth)
{
	RayTriangleIntersection intersection = Camera::getClosestIntersection(origin, rayDir, models);

	if (intersection.triangleIndex == -1)
	{
		return environment->sampleEnvironment(rayDir);
	}

	Model &model = *models[intersection.modelIndex];

	float u = intersection.u;
	float v = intersection.v;
	float w = 1 - u - v;

	ModelTriangle &tri = model.triangles->at(intersection.triangleIndex);
	Material* material = model.material;

	ModelVertex &v0 = model.verts->at(tri.vertices[0]);
	ModelVertex &v1 = model.verts->at(tri.vertices[1]);
	ModelVertex &v2 = model.verts->at(tri.vertices[2]);
	glm::vec3 normal = glm::normalize(v0.normal * w + v1.normal * u + v2.normal * v);
	glm::vec3 binormal = glm::normalize(v0.binormal * w + v1.binormal * u + v2.binormal * v);
	glm::vec3 tangent = glm::normalize(v0.tangent * w + v1.tangent * u + v2.tangent * v);
	glm::vec2 t0 = v0.texcoord;
	glm::vec2 t1 = v1.texcoord;
	glm::vec2 t2 = v2.texcoord;
	glm::vec2 texcoord = t0 * w + t1 * u + t2 * v;
	material->transformNormal(normal, binormal, tangent, texcoord.x, texcoord.y);
	
	if (currentDepth < MaxRayDepth)
	{
		if (model.material->mirror)
		{
			glm::vec3 reflectDir = glm::reflect(rayDir, normal);
			return renderRay(intersection.intersectionPoint, reflectDir, models, lights, currentDepth + 1);
		}
		else if (model.material->refract)
		{
			bool exiting = glm::dot(rayDir, normal) > 0;
			glm::vec3 refractDir = glm::refract(rayDir, exiting ? -normal : normal, exiting ? 1 / material->refractiveIndex : material->refractiveIndex);
			return renderRay(intersection.intersectionPoint, refractDir, models, lights, currentDepth + 1);
		}
	}

	glm::vec3 albedo = material->sampleAlbedo(texcoord.x, texcoord.y);
	return render(albedo, normal, intersection, rayDir, models, lights);
}

void Camera::initialiseGouraud(std::vector<Model*> &models, std::vector<Light> &lights, std::vector<std::vector<glm::vec3> > &vertexColours)
{
	glm::vec3 cameraPos = glm::vec3(posFromMatrix(this->cameraToWorld));

	for (int m = 0; m < models.size(); m++)
	{
		vertexColours.push_back(std::vector<glm::vec3>());
		for (int i = 0; i < models[m]->verts->size(); i++)
		{
			vertexColours[m].push_back(glm::vec3(0,0,0));
		}
	}

	for (int m = 0; m < models.size(); m++)
	{
		Model& model = *models[m];

		for (int i = 0; i < model.verts->size(); i++)
		{
			ModelVertex &vert = model.verts->at(i);
			glm::vec3 rayDir = glm::normalize(vert.pos - cameraPos);
			RayTriangleIntersection intersection = RayTriangleIntersection(vert.pos, -1, -1, m);
			glm::vec3 albedo = model.material->sampleAlbedo(vert.normal.x, vert.normal.y);
			
			vertexColours[m][i] = render(albedo, vert.normal, intersection, rayDir, models, lights);
		}
	}
	
}

Colour Camera::renderTracedGouraud(int x, int y, std::vector<Model*> &models, std::vector<Light> &lights, std::vector<std::vector<glm::vec3> > &vertexColours)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	glm::vec3 cameraPos = glm::vec3(posFromMatrix(this->cameraToWorld));
	
	RayTriangleIntersection intersection = Camera::getClosestIntersection(cameraPos, rayDir, models);

	if (intersection.triangleIndex == -1)
	{
		return vectorToColour(environment->sampleEnvironment(rayDir));
	}
	Model &model = *models[intersection.modelIndex];
	ModelTriangle tri = model.triangles->at(intersection.triangleIndex);

	float u = intersection.u;
	float v = intersection.v;
	float w = 1 - u - v;

	glm::vec3 v0 = vertexColours[intersection.modelIndex][tri.vertices[0]];
	glm::vec3 v1 = vertexColours[intersection.modelIndex][tri.vertices[1]];
	glm::vec3 v2 = vertexColours[intersection.modelIndex][tri.vertices[2]];
	return vectorToColour(v0 * w + v1 * u + v2 * v); 
}

void tonemapping(glm::vec3 &colour)
{
	colour.x = powf(colour.x, 0.4545) * 0.15f;
	colour.y = powf(colour.y, 0.4545) * 0.15f;
	colour.z = powf(colour.z, 0.4545) * 0.15f;
}

Colour Camera::renderTraced(int x, int y, std::vector<Model*> &models, std::vector<Light> &lights)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	glm::vec3 cameraPos = glm::vec3(posFromMatrix(this->cameraToWorld));
	glm::vec3 colour = renderRay(cameraPos, rayDir, models, lights);
	tonemapping(colour);
	return vectorToColour(colour);
}

void Camera::updateTransform()
{
	this->worldToCamera = glm::inverse(cameraToWorld);
}