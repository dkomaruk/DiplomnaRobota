#ifndef INFANTRY_H

#include "entity.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

using namespace glm;

struct InfantrySquad : public Entity
{
    int size;

    vec2 *soldierOffsets;

    Mesh *meshes;
    int *soldierMeshesId;
    int numOfMeshes;
};

void RenderInfantry(Entity *e, Game *game);
InfantrySquad CreateInfantrySquad(Mesh *soldierMeshes, int numOfMeshes, int squadSize);

#define INFANTRY_H
#endif