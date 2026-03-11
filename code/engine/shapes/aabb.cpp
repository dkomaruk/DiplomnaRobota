#include "aabb.h"

#include <glm/gtx/component_wise.hpp>

#include <algorithm>

void MergeAABB(AABB *dest, AABB *src)
{
    dest->min = glm::min(dest->min, src->min);
    dest->max = glm::max(dest->max, src->max);
}

void ExpandAABB(AABB *aabb, glm::vec3 point)
{
    aabb->min = glm::min(aabb->min, point);
    aabb->max = glm::max(aabb->max, point);
}

void UpdateAABBCorners(AABB *aabb)
{
    aabb->corners[0] = {aabb->min.x, aabb->min.y, aabb->min.z};
    aabb->corners[1] = {aabb->max.x, aabb->min.y, aabb->min.z};
    aabb->corners[2] = {aabb->min.x, aabb->max.y, aabb->min.z};
    aabb->corners[3] = {aabb->max.x, aabb->max.y, aabb->min.z};
    aabb->corners[4] = {aabb->min.x, aabb->min.y, aabb->max.z};
    aabb->corners[5] = {aabb->max.x, aabb->min.y, aabb->max.z};
    aabb->corners[6] = {aabb->min.x, aabb->max.y, aabb->max.z};
    aabb->corners[7] = {aabb->max.x, aabb->max.y, aabb->max.z};
}

void UpdateAABBMesh(AABB *aabb, Mesh *aabbMesh, bool recalculateCorners)
{
    if(recalculateCorners)
        UpdateAABBCorners(aabb);

    glBindBuffer(GL_ARRAY_BUFFER, aabbMesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(aabb->corners), aabb->corners);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

AABB TransformAABB(AABB *aabb, glm::mat4 transform)
{
    UpdateAABBCorners(aabb);

    AABB result = {glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)};
    for(int cornerIndex = 0; cornerIndex < ArrayCount(aabb->corners); cornerIndex++)
    {
        ExpandAABB(&result, glm::vec3(transform * glm::vec4(aabb->corners[cornerIndex], 1.0f)));
    }

    return result;
}
