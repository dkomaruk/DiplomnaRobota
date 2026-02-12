#ifndef AABB_H

#include "mesh.h"

#include <glm/vec3.hpp>

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 corners[8];
};

AABB TransformAABB(AABB *aabb, glm::mat4 transform);
void MergeAABB(AABB *dest, AABB *src);
void ExpandAABB(AABB *aabb, glm::vec3 point);

bool RayBoxIntersection(AABB *aabb, glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 *intersection);

void UpdateAABBCorners(AABB *aabb);
void UpdateAABBMesh(AABB *aabb, Mesh *aabbMesh, bool recalculateCorners = false);

#define AABB_H
#endif
