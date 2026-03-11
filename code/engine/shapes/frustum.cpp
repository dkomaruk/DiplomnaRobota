#include "frustum.h"

#include "game.h"

#include "line.h"
#include "plane.h"
#include "aabb.h"

Frustum CreateFrustum(glm::vec3 *near, glm::vec3 *far)
{
    Frustum frustum = {};

    frustum.planes[Frustum_L] = CreatePlane(near[Frustum_UL], near[Frustum_BL], far[Frustum_BL]);
    frustum.planes[Frustum_R] = CreatePlane(near[Frustum_BR], near[Frustum_UR], far[Frustum_UR]);
    frustum.planes[Frustum_U] = CreatePlane(near[Frustum_UR], near[Frustum_UL], far[Frustum_UL]);
    frustum.planes[Frustum_B] = CreatePlane(near[Frustum_BL], near[Frustum_BR], far[Frustum_BR]);
    frustum.planes[Frustum_N] = CreatePlane(near[Frustum_BL], near[Frustum_UL], near[Frustum_UR]);
    frustum.planes[Frustum_F] = CreatePlane(far[Frustum_UR], far[Frustum_UL], far[Frustum_BL]);

    return frustum;
}

void CreateFrustumLines(Line *lines, Line *normals, GLuint lineShader)
{
    for(int i = 0; i < 4; i++)
    {
        lines[i] = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), lineShader, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    for(int i = 0; i < 6; i++)
    {
        normals[i] = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), lineShader, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}
