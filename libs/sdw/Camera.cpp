#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "MatrixUtils.h"
#include "Camera.h"
#include "Light.h"
#include <iostream>

const int MaxRayDepth = 3;
const int MaxBounces = 10;
const float ImageSize = 50;
const float Exposure = 0.25f;

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
	glm::vec4 cPos = worldToCamera * vertexPosition;
	float u = ImageSize * focalLength * cPos.x / fabs(cPos.z) + width / 2.0f;
	float v = height / 2.0f - ImageSize * focalLength * cPos.y / fabs(cPos.z);

	return glm::vec3(u, v, -cPos.z);
}

glm::vec3 Camera::getRayDirection(float u, float v)
{
	float y = (v - (float)height / 2.0) / -(focalLength * ImageSize);
	float x = (u - (float)width / 2.0) / (focalLength * ImageSize);
	glm::vec4 localDir = glm::vec4(x, y, -1, 0);
	glm::vec4 dir = cameraToWorld * localDir;
	return glm::normalize(glm::vec3(dir));
}

RayTriangleIntersection Camera::getClosestIntersection(glm::vec3 &origin, glm::vec3 &rayDirection, std::vector<Model *> &models, int ignoreIndex)
{
	RayTriangleIntersection closestIntersection = RayTriangleIntersection(glm::vec3(0, 0, 0), 10000000, -1, -1);

	for (int m = 0; m < models.size(); m++)
	{
		Model &model = *models[m];
		if (!model.boundingBox.Hit(origin, rayDirection))
		{
			continue;
		}

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
			if (hit && possibleSolution.x < closestIntersection.distanceFromCamera && possibleSolution.x > 0.00001f)
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

glm::vec3 Camera::render(glm::vec3 &albedo, float metallic, float spec, glm::vec3 &specCol, glm::vec3 &normal, RayTriangleIntersection &intersection, glm::vec3 &rayDir, std::vector<Model *> &models, std::vector<Light> &lights)
{
	glm::vec3 lightIntensity = glm::vec3(0, 0, 0);
	// glm::vec3 ambientIntensity = glm::vec3(0.5f,0.5f,0.8f);
	for (int i = 0; i < lights.size(); i++)
	{
		glm::vec3 lightDir = lights[i].position - intersection.intersectionPoint;
		if (!Camera::inShadow(intersection, models, lightDir))
		{
			glm::vec3 lightCol = lights[i].colour * lights[i].lightAttenuation(lightDir);
			lightDir = glm::normalize(lightDir);
			lightIntensity += lightSurface(albedo, metallic, spec, specCol, rayDir, normal, lightDir, lightCol);
		}
	}

	// lightIntensity += ambientIntensity * albedo;
	return lightIntensity;
}

glm::vec3 Camera::lightSurface(glm::vec3 &albedo, float metallic, float spec, glm::vec3 &specCol, glm::vec3 &rayDir, glm::vec3 &normal, glm::vec3 &lightDir, glm::vec3 &lightIntensity)
{
	float ldn = glm::dot(lightDir, normal);
	ldn = ldn < 0 ? 0 : ldn;
	glm::vec3 diffuse = lightIntensity * ldn * albedo * (1 - metallic);
	glm::vec3 refl = glm::reflect(rayDir, normal);
	float rdl = glm::dot(lightDir, refl);
	rdl = rdl <= 0 ? 0 : rdl;
	glm::vec3 specular = lightIntensity * specCol * powf(rdl, spec);
	// return lightIntensity;
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
		// return environment->sampleEnvironment(rayDir);
		return glm::vec3(0,0,0);
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

	glm::vec3 albedo = material->sampleAlbedo(interpolated.texcoord.x, interpolated.texcoord.y);
	glm::vec3 specCol = glm::normalize(albedo);
	glm::vec3 reflectDir = glm::reflect(rayDir, interpolated.normal);
	if (currentDepth < MaxRayDepth)
	{
		if (model.material->refract)
		{
			float f = fresnel(rayDir, interpolated.normal, material->refractiveIndex);
			glm::vec3 refractDir = refract(rayDir, interpolated.normal, material->refractiveIndex);
			glm::vec3 reflectCol = renderRay(intersection.intersectionPoint, reflectDir, models, lights, currentDepth + 1, intersection.triangleIndex) * specCol;
			glm::vec3 surface = render(albedo, material->metallic, material->spec, specCol, interpolated.normal, intersection, rayDir, models, lights);
			return surface + renderRay(intersection.intersectionPoint, refractDir, models, lights, currentDepth + 1, intersection.triangleIndex) * albedo * (1 - f) + reflectCol * f;
		}
		else if (model.material->mirror)
		{
			float f = fresnel(rayDir, interpolated.normal, 1.35f);
			glm::vec3 surface = render(albedo, material->metallic, material->spec, specCol, interpolated.normal, intersection, rayDir, models, lights);
			return surface * (1 - f) + renderRay(intersection.intersectionPoint, reflectDir, models, lights, currentDepth + 1, intersection.triangleIndex) * specCol * f;
		}
	}

	float f = fresnel(rayDir, interpolated.normal, 1.35f);
	glm::vec3 env = environment->sampleEnvironment(reflectDir) * f * material->metallic * specCol;

	return render(albedo, material->metallic, material->spec, specCol, interpolated.normal, intersection, rayDir, models, lights) + env;
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

			glm::vec3 specCol = glm::normalize(albedo);

			vertexColours[m][i] = render(albedo, model.material->metallic, model.material->spec, specCol, vert.normal, intersection, rayDir, models, lights);
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

KdTree *Camera::renderPhotonMap(std::vector<Model *> &models, std::vector<Light> &lights, int iterations, float reflectMultiplier, const glm::vec3 &biasPosition, float biasAmount, float intensity)
{
	std::cout << "K Neighbours " << K_NEIGHBOURS << std::endl;
	std::vector<glm::vec3> *positions = new std::vector<glm::vec3>();
	std::vector<Photon> *intensities = new std::vector<Photon>();
	Photon photon;
	glm::vec3 averageLight = glm::vec3(0,0,0);
	for (int i = 0; i < lights.size(); i++)
	{
		averageLight += lights[i].position;
	}
	averageLight /= lights.size();
	
	glm::vec3 biasDirection = glm::normalize(biasPosition - averageLight) * biasAmount;

	for (int i = 0; i < iterations; i++)
	{
		Light &light = lights[(int)fmod(i, lights.size())];
		glm::vec3 dir = glm::normalize(biasDirection + glm::vec3((float)(rand()) / RAND_MAX - 0.5f, (float)(rand()) / RAND_MAX - 0.5f, (float)(rand()) / RAND_MAX - 0.5f));
		RayTriangleIntersection intersection = RayTriangleIntersection();
		glm::vec3 origin = light.position;
		glm::vec3 lightIntensity = light.colour / (float)iterations;
		float reflectProbability = 1;
		int bounces = 0;

		while (((float)(rand()) / RAND_MAX <= reflectProbability && bounces <= MaxBounces))
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
			glm::vec3 albedo = material->sampleAlbedo(interpolated.texcoord.x, interpolated.texcoord.y);
			origin = intersection.intersectionPoint;
			if (material->refract || material->mirror)
			{
				float f = fresnel(dir, interpolated.normal, material->refractiveIndex);
				lightIntensity *= albedo;
				if (material->mirror || ((float)(rand()) / RAND_MAX <= f))
				{
					dir = glm::reflect(dir, interpolated.normal);
				}
				else
				{
					dir = refract(dir, interpolated.normal, material->refractiveIndex);
				}
				reflectProbability = 1;
			}
			else
			{
				if (bounces > 0)
				{
					photon.intensity = lightIntensity * intensity;
					photon.dir = dir;
					positions->push_back(intersection.intersectionPoint);
					intensities->push_back(photon);
				}

				float metallic = material->metallic;
				if ((float)(rand()) / RAND_MAX <= (0.5f + metallic))
				{
					glm::vec3 specCol = glm::normalize(albedo);
					dir = glm::reflect(dir, interpolated.normal);
					reflectProbability = reflectMultiplier * (specCol.x + specCol.y + specCol.z) / 3; 
					lightIntensity *= (metallic * specCol + glm::vec3(1,1,1) * (1 - metallic));
				}
				else
				{
					//diffuse
					dir = glm::normalize(interpolated.normal * ((float)(rand()) / RAND_MAX) + interpolated.binormal * ((float)(rand()) / RAND_MAX - 0.5f) + interpolated.tangent * ((float)(rand()) / RAND_MAX - 0.5f));
					lightIntensity *= albedo;
					reflectProbability = reflectMultiplier * (albedo.x + albedo.y + albedo.z) / 3;
				}
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
	colour *= Exposure;
}

glm::vec3 Camera::renderRayBaked(glm::vec3 &origin, glm::vec3 &rayDir, std::vector<Model *> &models, std::vector<Light> &lights, KdTree *photonMap, int currentDepth, int ignoreIndex)
{
	RayTriangleIntersection intersection = Camera::getClosestIntersection(origin, rayDir, models, ignoreIndex);

	if (intersection.triangleIndex == -1)
	{
		return glm::vec3(0,0,0);
		// return environment->sampleEnvironment(rayDir);
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

	glm::vec3 albedo = material->sampleAlbedo(interpolated.texcoord.x, interpolated.texcoord.y);
	glm::vec3 specCol = glm::normalize(albedo);
	specCol = specCol * material->metallic + glm::vec3(1,1,1) * (1 - material->metallic);
	float f = fresnel(rayDir, interpolated.normal, material->refractiveIndex);
	glm::vec3 reflectDir = glm::reflect(rayDir, interpolated.normal);
	if (currentDepth <= MaxRayDepth)
	{
		if (material->refract)
		{
			glm::vec3 reflectCol = renderRayBaked(intersection.intersectionPoint, reflectDir, models, lights, photonMap, currentDepth + 1, intersection.triangleIndex) * specCol;
			glm::vec3 surface = render(albedo, material->metallic, material->spec, specCol, interpolated.normal, intersection, rayDir, models, lights);
			if (f < 1)
			{
				glm::vec3 refractDir = refract(rayDir, interpolated.normal, material->refractiveIndex);
				return surface * (1 - f) + renderRayBaked(intersection.intersectionPoint, refractDir, models, lights, photonMap, currentDepth + 1, intersection.triangleIndex) * (1 - f) * albedo + reflectCol * f;
			}
			return surface * (1 - f) + reflectCol;
		}
		else if (material->mirror)
		{
			glm::vec3 surface = render(albedo, material->metallic, material->spec, specCol, interpolated.normal, intersection, rayDir, models, lights);
			return surface + renderRayBaked(intersection.intersectionPoint, reflectDir, models, lights, photonMap, currentDepth + 1, intersection.triangleIndex) * specCol;
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
		specColour += colours[i].intensity * powf(rdl, material->spec) * dst;
	}
	colour /= areaOfSphere * 0.33f;

	glm::vec3 direct = render(albedo, material->metallic, material->spec, specCol, interpolated.normal, intersection, rayDir, models, lights);
	specColour += environment->sampleEnvironment(reflectDir) * f * material->metallic;

	return direct + (colour * albedo * (1 - material->metallic)) + specColour * specCol;
}

glm::vec3 Camera::renderTracedBaked(float x, float y, std::vector<Model *> &models, std::vector<Light> &lights, KdTree *photonMap)
{
	glm::vec3 rayDir = getRayDirection(x, y);
	glm::vec3 colour = renderRayBaked(cameraPosition, rayDir, models, lights, photonMap);
	tonemapping(colour);
	return colour;
}

glm::vec3 Camera::renderTraced(float x, float y, std::vector<Model *> &models, std::vector<Light> &lights)
{
	glm::vec3 rayDir = this->getRayDirection(x, y);
	glm::vec3 colour = renderRay(cameraPosition, rayDir, models, lights);
	tonemapping(colour);
	return colour;
}

void Camera::updateTransform()
{
	this->worldToCamera = glm::inverse(cameraToWorld);
	this->cameraPosition = glm::vec3(posFromMatrix(cameraToWorld));
}