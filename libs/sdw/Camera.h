#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "Light.h"
#include "ModelVertex.h"
#include "Model.h"

class Camera {
    public:
        glm::mat4 cameraToWorld;
        float focalLength;
        glm::vec3 getCanvasIntersectionPoint(glm::vec4 vertexPosition);
        void updateTransform();
        Colour renderTraced(int x, int y, std::vector<Model*> &models, std::vector<Light> &lights);
        Colour renderTracedGouraud(int x, int y, std::vector<Model*> &models, std::vector<Light> &lights, std::vector<glm::vec3> &vertexColours);
        void initialiseGouraud(std::vector<Model*> &models, std::vector<Light> &lights, std::vector<glm::vec3> &vertexColours);
        Camera();
        Camera(float focalLength, glm::mat4 cameraToWorld, int width, int height);
    private:
        static RayTriangleIntersection getClosestIntersection(glm::vec3 &origin, glm::vec3 &dir, std::vector<Model*> &models);
        static bool inShadow(RayTriangleIntersection &intersection, std::vector<Model*> &models, glm::vec3 &lightDir);
        glm::vec3 render(glm::vec3 &albedo, glm::vec3 &normal, RayTriangleIntersection &intersection, glm::vec3 &rayDir, std::vector<Model*> &models, std::vector<Light> &lights);
        glm::vec3 renderRay(glm::vec3 &origin, glm::vec3 &rayDir, std::vector<Model*> &models, std::vector<Light> &lights);
        glm::mat4 worldToCamera;
        int width;
        int height;
        glm::vec3 getRayDirection(float x, float y);
};
