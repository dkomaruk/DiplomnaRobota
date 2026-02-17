#ifndef FRUSTUM_H

#include "line.h"
#include "plane.h"

struct Game;

struct AABB;

enum FrustumCorner
{
    Frustum_BL,
    Frustum_BR,
    Frustum_UL,
    Frustum_UR,
};

enum FrustumPlane
{
    Frustum_L,
    Frustum_R,
    Frustum_U,
    Frustum_B,
    Frustum_N,
    Frustum_F
};

struct Frustum
{
    Plane planes[6];
};

Frustum CreateFrustum(glm::vec3 *nearPoints, glm::vec3 *farPoints);
void CreateFrustumLines(Line lines[4], Line normals[6], GLuint lineShader);

bool FrustumAABBIntersectionTest(Frustum* frustum, AABB *aabb);

#define FRUSTUM_H
#endif