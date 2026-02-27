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

enum IntersectionQuadrant
{
    IntersectionQuadrant_Right,
    IntersectionQuadrant_Left,
    IntersectionQuadrant_Middle
};

bool RayBoxIntersection(AABB *aabb, glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 *intersection)
{
    glm::vec3 candidatePlane;
	glm::ivec3 quadrant;
    glm::vec3 tMax;

	bool isInside = true;
	for(int i = 0; i < 3; i++)
    {
		if(rayOrigin[i] < aabb->min[i])
        {
			quadrant[i] = IntersectionQuadrant_Left;
			candidatePlane[i] = aabb->min[i];
			isInside = false;
		}
        else if(rayOrigin[i] > aabb->max[i])
        {
			quadrant[i] = IntersectionQuadrant_Right;
			candidatePlane[i] = aabb->max[i];
			isInside = false;
		}
        else
        {
			quadrant[i] = IntersectionQuadrant_Middle;
		}
    }

	if(isInside)
    {
		*intersection = rayOrigin;
		return true;
	}

	for(int i = 0; i < 3; i++)
    {
		if((quadrant[i] != IntersectionQuadrant_Middle) && (rayDirection[i] != 0.0f))
			tMax[i] = (candidatePlane[i] - rayOrigin[i]) / rayDirection[i];
		else
			tMax[i] = -1.0f;
    }

	int planeIndex = 0;
	for(int i = 1; i < 3; i++)
    {
		if(tMax[planeIndex] < tMax[i])
			planeIndex = i;
    }

	if(tMax[planeIndex] < 0.0f) return false;

    glm::vec3 intersectionPoint;
	for(int i = 0; i < 3; i++)
    {
		if(planeIndex != i)
        {
			intersectionPoint[i] = rayOrigin[i] + tMax[planeIndex] * rayDirection[i];
			if((intersectionPoint[i] < aabb->min[i]) || (intersectionPoint[i] > aabb->max[i]))
            {
                *intersection = intersectionPoint;
				return false;
            }
		}
        else
        {
			intersectionPoint[i] = candidatePlane[i];
		}
    }

    *intersection = intersectionPoint;

	return true;
}

bool RayBoxIntersection(AABB *aabb, glm::vec3 rayOrigin, glm::vec3 inverseRayDirection, float *intersectionDistance)
{
    glm::vec3 t0 = (aabb->min - rayOrigin) * inverseRayDirection;
    glm::vec3 t1 = (aabb->max - rayOrigin) * inverseRayDirection;

    glm::vec3 tMin = glm::min(t0, t1);
    glm::vec3 tMax = glm::max(t0, t1);

    float tPlaneEntry = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
    float tPlaneExit = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

    if(tPlaneEntry <= tPlaneExit && tPlaneExit >= 0.0f)
    {
        *intersectionDistance = tPlaneEntry;
        return true;
    }

    return false;
}