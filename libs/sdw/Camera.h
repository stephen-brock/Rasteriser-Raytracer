#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "Light.h"
#include "ModelVertex.h"
#include "Model.h"
#include "Environment.h"
#include "KdTree.h"

class Camera {
    public:
        glm::mat4 cameraToWorld;
        float focalLength;
        Environment* environment;
        glm::vec3 getCanvasIntersectionPoint(glm::vec4 vertexPosition);
        glm::vec3 getRayDirection(float x, float y);
        void updateTransform();
        Colour renderTraced(int x, int y, std::vector<Model*> &models, std::vector<Light> &lights);
        Colour renderTracedBaked(int x, int y, std::vector<Model*> &models, std::vector<Light> &lights, KdTree* photonMap);
        Colour renderTracedGouraud(int x, int y, std::vector<Model*> &models, std::vector<Light> &lights, std::vector<std::vector<glm::vec3> > &vertexColours);
        void initialiseGouraud(std::vector<Model*> &models, std::vector<Light> &lights, std::vector<std::vector<glm::vec3> > &vertexColours);
        KdTree* renderPhotonMap(std::vector<Model*> &models, std::vector<Light> &lights, int iterations, int bounces);
        Camera();
        Camera(float focalLength, glm::mat4 cameraToWorld, int width, int height, Environment* environment);
    private:
        static RayTriangleIntersection getClosestIntersection(glm::vec3 &origin, glm::vec3 &dir, std::vector<Model*> &models, int ignoreIndex = -1);
        static bool inShadow(RayTriangleIntersection &intersection, std::vector<Model*> &models, glm::vec3 &lightDir);
        glm::vec3 render(glm::vec3 &albedo, glm::vec3 &normal, RayTriangleIntersection &intersection, glm::vec3 &rayDir, std::vector<Model*> &models, std::vector<Light> &lights);
        glm::vec3 renderRay(glm::vec3 &origin, glm::vec3 &rayDir, std::vector<Model*> &models, std::vector<Light> &lights, int currentDepth=0, int ignoreIndex=-1);
        glm::vec3 renderRayBaked(glm::vec3 &origin, glm::vec3 &rayDir, std::vector<Model*> &models, std::vector<Light> &lights, KdTree* photonMap, int currentDepth=0, int ignoreIndex=-1);
        glm::vec3 refract(glm::vec3 &rayDir, glm::vec3 &normal, float ior);
        float fresnel(glm::vec3 &rayDir, glm::vec3 &normal, float n0);
        glm::mat4 worldToCamera;
        int width;
        int height;
};
