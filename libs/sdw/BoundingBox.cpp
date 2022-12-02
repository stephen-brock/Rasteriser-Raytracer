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
        float tmp = tF;
        tF = tT;
        tF = tmp;
        // std::swap(tF, tT);
    }

    float tFy = fromDir.y / dir.y;
    float tTy = toDir.y / dir.y;

    if (tFy > tTy)
    {
        float tmp = tFy;
        tFy = tTy;
        tFy = tmp;
        // std::swap(tFy, tTy);
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
        float tmp = tFz;
        tFz = tTz;
        tFz = tmp;
        // std::swap(tFz, tTz);
    }

    if ((tF > tTz) || (tFz > tT)) 
    {
        return false; 
    }

    return true;
}

