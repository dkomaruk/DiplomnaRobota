#ifndef PLANE_H

#include <glm/vec3.hpp>

struct Plane
{
    float d;
    glm::vec3 normal;
};

Plane CreatePlane(glm::vec3 a, glm::vec3 b, glm::vec3 c);
float PointPlaneDistance(Plane *plane, glm::vec3 point);

#define PLANE_H
#endif