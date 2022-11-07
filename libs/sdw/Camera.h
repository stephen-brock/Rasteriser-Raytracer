#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ModelTriangle.h"
#include "RayTriangleIntersection.h"

class Camera {
    public:
        glm::mat4 cameraToWorld;
        float focalLength;
        glm::vec3 getCanvasIntersectionPoint(glm::vec4 vertexPosition);
        RayTriangleIntersection getClosestIntersection(glm::vec3 dir, std::vector<ModelTriangle> &triangles);
        void updateTransform();
        Colour renderTraced(int x, int y, std::vector<ModelTriangle> &triangles);
        Camera();
        Camera(float focalLength, glm::mat4 cameraToWorld, int width, int height);
    private:
        glm::mat4 worldToCamera;
        int width;
        int height;
        glm::vec3 getRayDirection(int x, int y);
};
