#include "ray.h"

#include "game.h"

Ray CastPickingRay(Game *game, glm::vec2 mousePos)
{
    Ray pickingRay = {};

    glm::vec3 windowPos = glm::vec3(mousePos, 0.0f);

    glm::vec3 rayNear = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                        glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));
    windowPos.z = 1.0f;
    glm::vec3 rayFar = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                        glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));

    pickingRay.origin = game->camera.position;
    pickingRay.direction = glm::normalize(rayFar - rayNear);
    pickingRay.inverseDirection = 1.0f / pickingRay.direction;

    return pickingRay;
}