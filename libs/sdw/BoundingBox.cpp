#include "BoundingBox.h"

const float Padding = 0.001f;

BoundingBox::BoundingBox() = default;
BoundingBox::BoundingBox(glm::vec3 from, glm::vec3 to)
{
    this->from = from - glm::vec3(Padding, Padding, Padding);
    this->to = to + glm::vec3(Padding, Padding, Padding);
}

bool BoundingBox::Hit(glm::vec3 &origin, glm::vec3 &dir)
{
    glm::vec3 fromDir = from - origin;
    glm::vec3 toDir = to - origin;

    float tF = fromDir.x / dir.x;
    float tT = toDir.x / dir.x;

    if (tF > tT)
    {
        // std::swap(tF, tT);
        float tmp = tF;
        tF = tT;
        tT = tmp;
    }

    float tFy = fromDir.y / dir.y;
    float tTy = toDir.y / dir.y;

    if (tFy > tTy)
    {
        // std::swap(tFy, tTy);
        float tmp = tFy;
        tFy = tTy;
        tTy = tmp;
    }

    if ((tF > tTy) || (tFy > tT)) 
    {
        return false; 
    }

    tF = fmax(tF, tFy);
    tT = fmin(tT, tTy);

    float tFz = fromDir.z / dir.z;
    float tTz = toDir.z / dir.z;

    if (tFz > tTz)
    {
        // std::swap(tFz, tTz);
        float tmp = tFz;
        tFz = tTz;
        tTz = tmp;
    }

    if ((tF > tTz) || (tFz > tT)) 
    {
        return false; 
    }

    return true;
}

