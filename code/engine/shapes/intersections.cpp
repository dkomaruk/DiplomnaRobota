#include "intersections.h"

#include "aabb.h"
#include "frustum.h"
#include "ray.h"

bool RayBoxIntersection(AABB *aabb, Ray *ray, float *intersectionDistance)
{
    glm::vec3 t0 = (aabb->min - ray->origin) * ray->inverseDirection;
    glm::vec3 t1 = (aabb->max - ray->origin) * ray->inverseDirection;

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

enum IntersectionQuadrant
{
    IntersectionQuadrant_Right,
    IntersectionQuadrant_Left,
    IntersectionQuadrant_Middle
};

bool RayBoxIntersection(AABB *aabb, Ray *ray, glm::vec3 *intersection)
{
    glm::vec3 candidatePlane;
	glm::ivec3 quadrant;
    glm::vec3 tMax;

	bool isInside = true;
	for(int i = 0; i < 3; i++)
    {
		if(ray->origin[i] < aabb->min[i])
        {
			quadrant[i] = IntersectionQuadrant_Left;
			candidatePlane[i] = aabb->min[i];
			isInside = false;
		}
        else if(ray->origin[i] > aabb->max[i])
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
		*intersection = ray->origin;
		return true;
	}

	for(int i = 0; i < 3; i++)
    {
		if((quadrant[i] != IntersectionQuadrant_Middle) && (ray->direction[i] != 0.0f))
			tMax[i] = (candidatePlane[i] - ray->origin[i]) / ray->direction[i];
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
			intersectionPoint[i] = ray->origin[i] + tMax[planeIndex] * ray->direction[i];
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

bool FrustumAABBIntersectionTest(Frustum* frustum, AABB *aabb)
{
    glm::vec3 center = (aabb->max + aabb->min) * 0.5f;
    glm::vec3 extents = aabb->max - center;

    for(int planeIndex = 0; planeIndex < 6; planeIndex++)
    {
        float projectionLength = glm::dot(extents, glm::abs(frustum->planes[planeIndex].normal));
        float distanceToPlane = PointPlaneDistance(&frustum->planes[planeIndex], center);

        if(distanceToPlane < -projectionLength)
            return false;
    }

    return true;
}