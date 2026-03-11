#ifndef INTERSECTIONS_H

#include <glm/vec3.hpp>

struct Ray;
struct AABB;
struct Frustum;

bool RayBoxIntersection(AABB *aabb, Ray *ray, float *intersectionDistance);
bool RayBoxIntersection(AABB *aabb, Ray *ray, glm::vec3 *intersection);

bool FrustumAABBIntersectionTest(Frustum* frustum, AABB *aabb);

#define INTERSECTIONS_H
#endif