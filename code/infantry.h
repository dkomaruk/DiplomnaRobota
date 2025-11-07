#ifndef INFANTRY_H

#include "entity.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct InfantrySquad : public Entity
{
    int size;

    vec2 *soldierOffsets;

    int *soldierModelsId;
};

void RenderInfantry(Entity *e, Game *game);
InfantrySquad CreateInfantrySquad(Model *models, int numOfModels, int squadSize);

#define INFANTRY_H
#endif