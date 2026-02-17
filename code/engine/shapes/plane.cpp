#include "plane.h"

#include <glm/gtc/matrix_transform.hpp>

Plane CreatePlane(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    Plane result = {};

    result.normal = glm::normalize(glm::cross(b - a, c - a));
    result.d = -glm::dot(result.normal, a);

    return result;
}

float PointPlaneDistance(Plane *plane, glm::vec3 point)
{
    return glm::dot(plane->normal, point) + plane->d;
}