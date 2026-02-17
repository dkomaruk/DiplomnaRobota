#ifndef RAY_H

struct Game;

#include <glm/vec3.hpp>

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 inverseDirection;
};

Ray CastPickingRay(Game *game, glm::vec2 mousePos);

#define RAY_H
#endif