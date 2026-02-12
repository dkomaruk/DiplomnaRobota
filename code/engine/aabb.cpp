#include "aabb.h"

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

/*
bool RayBoxIntersection(float minB[NUMDIM], float maxB[NUMDIM], float origin[NUMDIM], float dir[NUMDIM], float coord[NUMDIM])
{
	char inside = true;
	char quadrant[NUMDIM];
	register int i;
	float maxT[NUMDIM];
	float candidatePlane[NUMDIM];

	for(i=0; i < NUMDIM; i++)
    {
		if(origin[i] < minB[i])
        {
			quadrant[i] = LEFT;
			candidatePlane[i] = minB[i];
			inside = false;
		}
        else if(origin[i] > maxB[i])
        {
			quadrant[i] = RIGHT;
			candidatePlane[i] = maxB[i];
			inside = false;
		}
        else
        {
			quadrant[i] = MIDDLE;
		}

    }

	if(inside)
    {
		coord = origin;
		return true;
	}

	for(i = 0; i < NUMDIM; i++)
    {
		if(quadrant[i] != MIDDLE && dir[i] != 0.)
			maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
		else
			maxT[i] = -1.;
    }


	int whichPlane = 0;
	for(i = 1; i < NUMDIM; i++)
    {
		if(maxT[whichPlane] < maxT[i])
			whichPlane = i;
    }

	if(maxT[whichPlane] < 0.) return false;

	for(i = 0; i < NUMDIM; i++)
    {
		if(whichPlane != i)
        {
			coord[i] = origin[i] + maxT[whichPlane] * dir[i];
			if(coord[i] < minB[i] || coord[i] > maxB[i])
				return false;
		}
        else
        {
			coord[i] = candidatePlane[i];
		}
    }

	return true;
}
*/
enum
{
    RIGHT,
    LEFT,
    MIDDLE
};

bool RayBoxIntersection(AABB *aabb, glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 *intersection)
{
	bool isInside = true;
	int quadrant[3];
	float maxT[3];
	float candidatePlane[3];

	for(int i = 0; i < 3; i++)
    {
		if(rayOrigin[i] < aabb->min[i])
        {
			quadrant[i] = LEFT;
			candidatePlane[i] = aabb->min[i];
			isInside = false;
		}
        else if(rayOrigin[i] > aabb->max[i])
        {
			quadrant[i] = RIGHT;
			candidatePlane[i] = aabb->max[i];
			isInside = false;
		}
        else
        {
			quadrant[i] = MIDDLE;
		}

    }

	if(isInside)
    {
		*intersection = rayOrigin;
		return true;
	}

	for(int i = 0; i < 3; i++)
    {
		if(quadrant[i] != MIDDLE && rayDirection[i] != 0.0f)
			maxT[i] = (candidatePlane[i] - rayOrigin[i]) / rayDirection[i];
		else
			maxT[i] = -1.0f;
    }


	int whichPlane = 0;
	for(int i = 1; i < 3; i++)
    {
		if(maxT[whichPlane] < maxT[i])
			whichPlane = i;
    }

	if(maxT[whichPlane] < 0.0f) return false;

    glm::vec3 intersectionPoint;
	for(int i = 0; i < 3; i++)
    {
		if(whichPlane != i)
        {
			intersectionPoint[i] = rayOrigin[i] + maxT[whichPlane] * rayDirection[i];
			if(intersectionPoint[i] < aabb->min[i] || intersectionPoint[i] > aabb->max[i])
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
