#include "RayTriangleIntersection.h"

RayTriangleIntersection::RayTriangleIntersection() = default;
RayTriangleIntersection::RayTriangleIntersection(const glm::vec3 &point, float distance, size_t index, size_t modelIndex) :
		intersectionPoint(point),
		distanceFromCamera(distance),
		triangleIndex(index), modelIndex(modelIndex) {}


std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection) {
	os << "Intersection is at [" << intersection.intersectionPoint[0] << "," << intersection.intersectionPoint[1] << "," <<
	   intersection.intersectionPoint[2] << "] on triangle " << intersection.triangleIndex <<
	   " at a distance of " << intersection.distanceFromCamera;
	return os;
}
