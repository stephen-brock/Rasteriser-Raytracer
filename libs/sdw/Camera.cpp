#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "MatrixUtils.h"
#include "Camera.h"
#include "Light.h"
#include <iostream>

const int MaxRayDepth = 4;
const int MaxBounces = 5;

Camera::Camera()
{
	width = 0;
	height = 0;
}

Camera::Camera(float focalLength, glm::mat4 cameraToWorld, int width, int height, Environment *environment)
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
	float u = focalLength * -cPos.x / fabs(cPos.z) + width / 2;
	float v = height / 2 - focalLength * cPos.y / fabs(cPos.z);

	return glm::vec3(u, v, cPos.z);
}

glm::vec3 Camera::getRayDirection(float u, float v)
{
	float y = (v - (float)height / 2.0) / -focalLength;
	float x = (u - (float)width / 2.0) / focalLength;
	glm::vec4 localDir = glm::vec4(-x, y, -1, 0);
	glm::vec4 dir = localDir * cameraToWorld;
	return glm::normalize(glm::vec3(dir));
}

RayTriangleIntersection Camera::getClosestIntersection(glm::vec3 &origin, glm::vec3 &rayDirection, std::vector<Model *> &models, int ignoreIndex)
{
	RayTriangleIntersection closestIntersection = RayTriangleIntersection(glm::vec3(0, 0, 0), 10000000, -1, -1);

	for (int m = 0; m < models.size(); m++)
	{
		Model &model = *models[m];

		for (int i = 0; i < model.triangles->size(); i++)
		{
			ModelTriangle triangle = model.triangles->at(i);
			glm::vec3 v0 = model.transformedVerts->at(triangle.vertices[0]).pos;
			glm::vec3 v1 = model.transformedVerts->at(triangle.vertices[1]).pos;
			glm::vec3 v2 = model.transformedVerts->at(triangle.vertices[2]).pos;
			glm::vec3 e0 = v1 - v0;
			glm::vec3 e1 = v2 - v0;
			glm::vec3 SPVector = origin - v0;
			glm::mat3 DEMatrix(-rayDirection, e0, e1);
			glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
			bool hit = (possibleSolution.x > 0) && (possibleSolution.y >= 0.0) && (possibleSolution.y <= 1.0) && (possibleSolution.z >= 0.0) && (possibleSolution.z <= 1.0) && (possibleSolution.y + possibleSolution.z) <= 1.0;
			if (hit && possibleSolution.x < closestIntersection.distanceFromCamera && possibleSolution.x > 0.01f)
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

bool Camera::inShadow(RayTriangleIntersection &intersection, std::vector<Model *> &models, glm::vec3 &lightDir)
{
	RayTriangleIntersection shadowIntersection = Camera::getClosestIntersection(intersection.intersectionPoint, lightDir, models, intersection.triangleIndex);
	if (shadowIntersection.triangleIndex != -1 && shadowIntersection.distanceFromCamera < 1)
	{
		return true;
	}
	return false;
}

glm::vec3 Camera::render(glm::vec3 &albedo, glm::vec3 &normal, RayTriangleIntersection &intersection, glm::vec3 &rayDir, std::vector<Model *> &models, std::vector<Light> &lights)
{
	glm::vec3 lightIntensity = glm::vec3(0, 0, 0);
	// glm::vec3 ambientIntensity = glm::vec3(0.1f,0.f,0.2f);
	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 lightDir = lights[i].position - intersection.intersectionPoint;
		if (!Camera::inShadow(intersection, models, lightDir))
		{
			glm::vec3 lightCol = lights[i].colour * lights[i].lightAttenuation(lightDir);
			lightDir = glm::normalize(lightDir);
			lightIntensity += lightSurface(albedo, rayDir, normal, lightDir, lightCol);
		}
	}

	// lightIntensity += ambientIntensity;
	return lightIntensity;
}

glm::vec3 Camera::lightSurface(glm::vec3 &albedo, glm::vec3 &rayDir, glm::vec3 &normal, glm::vec3 &lightDir, glm::vec3 &lightIntensity)
{
	float ldn = glm::dot(lightDir, normal);
	ldn = ldn < 0 ? 0 : ldn;
	glm::vec3 diffuse = lightIntensity * ldn * albedo;
	glm::vec3 refl = glm::reflect(rayDir, normal);
	float rdl = glm::dot(lightDir, refl);
	rdl = rdl <= 0 ? 0 : rdl;
	glm::vec3 specular = lightIntensity * powf(rdl, 8);

	return diffuse + specular;
}

//used code/logic from https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
float Camera::fresnel(glm::vec3 &rayDir, glm::vec3 &normal, float n0)
{
	float dot = glm::dot(rayDir, normal);
	float n1 = 1;
	if (dot > 0)
	{
		n1 = n0;
		n0 = 1;
	}

	float tir = n0 / n1 * sqrtf(1 - dot * dot);
	if (tir >= 1)
	{
		return 1;
	}
	else
	{
		float cosX = sqrtf(1 - tir * tir);
		dot = fabs(dot);
		float rs = (n0 * dot - n1 * cosX) / (n0 * dot + n1 * cosX);
		float rp = (n1 * dot - n0 * cosX) / (n1 * dot + n0 * cosX);
		return (rs * rs + rp * rp) / 2;
	}
}

//used code/logic from https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
glm::vec3 Camera::refract(glm::vec3 &rayDir, glm::vec3 &normal, float ior)
{
	glm::vec3 nrm = normal;
	float dot = glm::dot(rayDir, normal);
	float etai = 1;
	float etat = ior;
	if (dot < 0)
	{
		dot = -dot;
	}
	else 
	{
		std::swap(etai, etat);
		nrm = -nrm;
	}

	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - dot * dot);

	return k < 0 ? glm::vec3(0,0,0) : eta * rayDir + (eta * dot - sqrtf(k)) * nrm;
}

void Camera::interpolateVertexData(ModelVertex &v0, ModelVertex &v1, ModelVertex &v2, ModelVertex &out, float u, float v)
{
	float w = 1 - u - v;
	out.normal = glm::normalize(v0.normal * w + v1.normal * u + v2.normal * v);
	out.binormal = glm::normalize(v0.binormal * w + v1.binormal * u + v2.binormal * v);
	out.tangent = glm::normalize(v0.tangent * w + v1.tangent * u + v2.tangent * v);

	glm::vec2 t0 = v0.texcoord;
	glm::vec2 t1 = v1.texcoord;
	glm::vec2 t2 = v2.texcoord;
	out.texcoord = t0 * w + t1 * u + t2 * v;
}

glm::vec3 Camera::renderRay(glm::vec3 &origin, glm::vec3 &rayDir, std::vector<Model *> &models, std::vector<Light> &lights, int currentDepth, int ignoreIndex)
{
	RayTriangleIntersection intersection = Camera::getClosestIntersection(origin, rayDir, models, ignoreIndex);

	if (intersection.triangleIndex == -1)
	{
		return environment->sampleEnvironment(rayDir);
	}

	Model &model = *models[intersection.modelIndex];

	float u = intersection.u;
	float v = intersection.v;

	ModelTriangle &tri = model.triangles->at(intersection.triangleIndex);
	Material *material = model.material;

	ModelVertex &v0 = model.transformedVerts->at(tri.vertices[0]);
	ModelVertex &v1 = model.transformedVerts->at(tri.vertices[1]);
	ModelVertex &v2 = model.transformedVerts->at(tri.vertices[2]);
	ModelVertex interpolated;
	interpolateVertexData(v0, v1, v2, interpolated, u, v);
	material->transformNormal(interpolated.normal, interpolated.binormal, interpolated.tangent, interpolated.texcoord.x, interpolated.texcoord.y);

	if (currentDepth < MaxRayDepth)
	{
		if (model.material->refract)
		{
			float f = fresnel(rayDir, interpolated.normal, material->refractiveIndex);
			glm::vec3 refractDir = refract(rayDir, interpolated.normal, material->refractiveIndex);
			glm::vec3 reflectDir = glm::reflect(rayDir, interpolated.normal);
			glm::vec3 reflectCol = renderRay(intersection.intersectionPoint, reflectDir, models, lights, currentDepth + 1, intersection.triangleIndex);
			return renderRay(intersection.intersectionPoint, refractDir, models, lights, currentDepth + 1, intersection.triangleIndex) * (1 - f) + reflectCol * f;
		}
		else if (model.material->mirror)
		{
			glm::vec3 reflectDir = glm::reflect(rayDir, interpolated.normal);
			return renderRay(intersection.intersectionPoint, reflectDir, models, lights, currentDepth + 1, intersection.triangleIndex);
		}
	}

	glm::vec3 albedo = material->sampleAlbedo(interpolated.texcoord.x, interpolated.texcoord.y);
	return render(albedo, interpolated.normal, intersection, rayDir, models, lights);
}

void Camera::initialiseGouraud(std::vector<Model *> &models, std::vector<Light> &lights, std::vector<std::vector<glm::vec3> > &vertexColours)
{
	for (int m = 0; m < models.size(); m++)
	{
		vertexColours.push_back(std::vector<glm::vec3>());
		for (int i = 0; i < models[m]->VertexAmount(); i++)
		{
			vertexColours[m].push_back(glm::vec3(0, 0, 0));
		}
	}

	for (int m = 0; m < models.size(); m++)
	{
		Model &model = *models[m];

		for (int i = 0; i < model.VertexAmount(); i++)
		{
			ModelVertex &vert = model.transformedVerts->at(i);
			glm::vec3 rayDir = glm::normalize(vert.pos - cameraPosition);
			RayTriangleIntersection intersection = RayTriangleIntersection(vert.pos, -1, -1, m);
			glm::vec3 albedo = model.material->sampleAlbedo(vert.normal.x, vert.normal.y);

			vertexColours[m][i] = render(albedo, vert.normal, intersection, rayDir, models, lights);
		}
	}
}

Colour Camera::renderTracedGouraud(int x, int y, std::vector<Model *> &models, std::vector<Light> &lights, std::vector<std::vector<glm::vec3> > &vertexColours)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);

	RayTriangleIntersection intersection = Camera::getClosestIntersection(cameraPosition, rayDir, models);

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

KdTree *Camera::renderPhotonMap(std::vector<Model *> &models, std::vector<Light> &lights, int iterations, float materialAbsorbProbability)
{
	std::cout << "K Neighbours " << K_NEIGHBOURS << std::endl;
	std::vector<glm::vec3> *positions = new std::vector<glm::vec3>();
	std::vector<Photon> *intensities = new std::vector<Photon>();
	Photon photon;

	for (int i = 0; i < iterations; i++)
	{
		Light &light = lights[(int)fmod(i, lights.size())];
		glm::vec3 dir = glm::normalize(glm::vec3((float)(rand()) / RAND_MAX - 0.5f, (float)(rand()) / RAND_MAX - 0.5f, (float)(rand()) / RAND_MAX - 0.5f));
		RayTriangleIntersection intersection = RayTriangleIntersection();
		glm::vec3 origin = light.position;
		glm::vec3 lightIntensity = light.colour * (1 - materialAbsorbProbability) / (float)iterations;
		float absorbProbability = 0;
		int bounces = 0;

		while (((float)(rand()) / RAND_MAX >= absorbProbability && bounces <= MaxBounces) || bounces <= 1)
		{
			intersection = getClosestIntersection(origin, dir, models, intersection.triangleIndex);
			if (intersection.modelIndex == -1)
			{
				break;
			}

			Model &model = *models[intersection.modelIndex];
			Material *material = model.material;

			float u = intersection.u;
			float v = intersection.v;

			ModelTriangle &tri = model.triangles->at(intersection.triangleIndex);

			ModelVertex &v0 = model.transformedVerts->at(tri.vertices[0]);
			ModelVertex &v1 = model.transformedVerts->at(tri.vertices[1]);
			ModelVertex &v2 = model.transformedVerts->at(tri.vertices[2]);
			ModelVertex interpolated;
			interpolateVertexData(v0, v1, v2, interpolated, u, v);
			material->transformNormal(interpolated.normal, interpolated.binormal, interpolated.tangent, interpolated.texcoord.x, interpolated.texcoord.y);

			dir = glm::normalize(intersection.intersectionPoint - origin);

			origin = intersection.intersectionPoint;
			if (material->refract || material->mirror)
			{
				float f = fresnel(dir, interpolated.normal, material->refractiveIndex);
				if (material->mirror || ((float)(rand()) / RAND_MAX <= f))
				{
					dir = glm::reflect(dir, interpolated.normal);
				}
				else
				{
					dir = refract(dir, interpolated.normal, material->refractiveIndex);
				}
				absorbProbability = 0;
			}
			else
			{
				if (bounces > 0)
				{
					photon.intensity = lightIntensity;
					photon.dir = dir;
					positions->push_back(intersection.intersectionPoint);
					intensities->push_back(photon);
				}

				float f = fresnel(dir, interpolated.normal, 1.35);
				if ((float)(rand()) / RAND_MAX <= f)
				{
					dir = glm::reflect(dir, interpolated.normal);
				}
				else
				{
					//diffuse
					dir = glm::normalize(interpolated.normal * ((float)(rand()) / RAND_MAX) + interpolated.binormal * ((float)(rand()) / RAND_MAX - 0.5f) + interpolated.tangent * ((float)(rand()) / RAND_MAX - 0.5f));
					lightIntensity *= material->sampleAlbedo(interpolated.texcoord.x, interpolated.texcoord.y);
				}

				absorbProbability = materialAbsorbProbability;
			}

			bounces++;
		}
	}

	std::cout << intensities->size() << std::endl;

	KdTree *photonMap = new KdTree(*positions, *intensities);
	delete positions;
	delete intensities;

	return photonMap;
}

void tonemapping(glm::vec3 &colour)
{
	colour.x = powf(fmax(0, colour.x), 0.4545);
	colour.y = powf(fmax(0, colour.y), 0.4545);
	colour.z = powf(fmax(0, colour.z), 0.4545);
	colour *= 0.2f;
}

glm::vec3 Camera::renderRayBaked(glm::vec3 &origin, glm::vec3 &rayDir, std::vector<Model *> &models, std::vector<Light> &lights, KdTree *photonMap, int currentDepth, int ignoreIndex)
{
	RayTriangleIntersection intersection = Camera::getClosestIntersection(origin, rayDir, models, ignoreIndex);

	if (intersection.triangleIndex == -1)
	{
		return environment->sampleEnvironment(rayDir);
	}

	Model &model = *models[intersection.modelIndex];
	Material *material = model.material;

	float u = intersection.u;
	float v = intersection.v;

	ModelTriangle &tri = model.triangles->at(intersection.triangleIndex);
	ModelVertex &v0 = model.transformedVerts->at(tri.vertices[0]);
	ModelVertex &v1 = model.transformedVerts->at(tri.vertices[1]);
	ModelVertex &v2 = model.transformedVerts->at(tri.vertices[2]);
	ModelVertex interpolated;
	interpolateVertexData(v0, v1, v2, interpolated, u, v);
	material->transformNormal(interpolated.normal, interpolated.binormal, interpolated.tangent, interpolated.texcoord.x, interpolated.texcoord.y);

	if (currentDepth <= MaxRayDepth)
	{
		if (material->refract)
		{
			float f = fresnel(rayDir, interpolated.normal, material->refractiveIndex);
			glm::vec3 refractDir = refract(rayDir, interpolated.normal, material->refractiveIndex);
			glm::vec3 reflectDir = glm::reflect(rayDir, interpolated.normal);
			glm::vec3 reflectCol = renderRayBaked(intersection.intersectionPoint, reflectDir, models, lights, photonMap, currentDepth + 1, intersection.triangleIndex);
			return renderRayBaked(intersection.intersectionPoint, refractDir, models, lights, photonMap, currentDepth + 1, intersection.triangleIndex) * (1 - f) + reflectCol * f;
		}
		else if (material->mirror)
		{
			glm::vec3 reflectDir = glm::reflect(rayDir, interpolated.normal);
			return renderRayBaked(intersection.intersectionPoint, reflectDir, models, lights, photonMap, currentDepth + 1, intersection.triangleIndex);
		}
	}

	std::array<float, K_NEIGHBOURS> sqrDistances;
	std::array<Photon, K_NEIGHBOURS> colours = photonMap->SearchKNeighbours(intersection.intersectionPoint, sqrDistances);

	float areaOfSphere = sqrDistances[K_NEIGHBOURS - 1] * M_PI;
	float r = sqrtf(sqrDistances[K_NEIGHBOURS - 1]);
	glm::vec3 colour = glm::vec3(0, 0, 0);
	glm::vec3 specColour = glm::vec3(0, 0, 0);
	for (int i = 0; i < colours.size(); i++)
	{
		//cone filtering
		float dst = fmax(0, 1 - sqrtf(sqrDistances[i] / r));
		float dot = glm::dot(-colours[i].dir, interpolated.normal);
		glm::vec3 refl = glm::reflect(rayDir, interpolated.normal);
		float rdl = glm::dot(-colours[i].dir, refl);
		rdl = rdl <= 0 ? 0 : rdl;
		colour += (colours[i].intensity * (float)fmax(0, dot)) * dst;
		specColour += colours[i].intensity * powf(rdl, 8) * dst;
	}
	colour /= areaOfSphere * 0.33f;

	glm::vec3 albedo = material->sampleAlbedo(interpolated.texcoord.x, interpolated.texcoord.y);
	glm::vec3 direct = render(albedo, interpolated.normal, intersection, rayDir, models, lights);

	return direct + colour * albedo + specColour;
}

Colour Camera::renderTracedBaked(int x, int y, std::vector<Model *> &models, std::vector<Light> &lights, KdTree *photonMap)
{
	glm::vec3 rayDir = getRayDirection(x, y);
	glm::vec3 colour = renderRayBaked(cameraPosition, rayDir, models, lights, photonMap);
	tonemapping(colour);
	return vectorToColour(colour);
}

Colour Camera::renderTraced(int x, int y, std::vector<Model *> &models, std::vector<Light> &lights)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	glm::vec3 colour = renderRay(cameraPosition, rayDir, models, lights);
	tonemapping(colour);
	return vectorToColour(colour);
}

void Camera::updateTransform()
{
	this->worldToCamera = glm::inverse(cameraToWorld);
	this->cameraPosition = glm::vec3(posFromMatrix(cameraToWorld));
}