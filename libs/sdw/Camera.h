#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"
#include "Light.h"

class Camera {
    public:
        glm::mat4 cameraToWorld;
        float focalLength;
        glm::vec3 getCanvasIntersectionPoint(glm::vec4 vertexPosition);
        void updateTransform();
        Colour renderTraced(int x, int y, std::vector<ModelTriangle> &triangles, std::vector<Light> &lights);
        Camera();
        Camera(float focalLength, glm::mat4 cameraToWorld, int width, int height);
    private:
        static RayTriangleIntersection getClosestIntersection(glm::vec3 origin, glm::vec3 dir, std::vector<ModelTriangle> &triangles);
        static bool inShadow(RayTriangleIntersection &intersection, std::vector<ModelTriangle> &triangles, std::vector<Light> &lights);
        glm::mat4 worldToCamera;
        int width;
        int height;
        glm::vec3 getRayDirection(float x, float y);
};
