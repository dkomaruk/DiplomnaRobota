#include "infantry.h"

#include "defines.h"
#include "game.h"
#include "shader.h"

void RenderInfantry(Entity *e, Game *game)
{
    Assert(e->type == EntityType_Infantry)
    InfantrySquad *squad = (InfantrySquad *)(e);

    if(game->pickingPass)
    {
        ShaderSetUInt(game->pickingShader, "u_objectIndex", e->id);
    }

    for(int i = 0; i < squad->size; i++)
    {
        glm::vec3 offset = glm::vec3(squad->soldierOffsets[i].x, 0.0f, squad->soldierOffsets[i].y);
        glm::mat4 modelMat = PrepareModelMatrix(squad->position + offset, squad->rotation, squad->scale);
        RenderModel(game, &squad->models[squad->soldierModelsId[i]], modelMat);
    }
}

InfantrySquad CreateInfantrySquad(Model *soldierModels, int numOfModels, int squadSize)
{
    InfantrySquad squad = {};

    squad.type = EntityType_Infantry;
    squad.size = squadSize;
    squad.models = soldierModels;
    squad.numOfModels = numOfModels;
    squad.Render = RenderInfantry;

    squad.soldierModelsId = (int *)malloc(sizeof(int) * squadSize);
    squad.soldierOffsets = (glm::vec2 *)malloc(sizeof(glm::vec2) * squadSize);
    for(int i = 0; i < squadSize; i++)
    {
        squad.soldierModelsId[i] = i % numOfModels;
        squad.soldierOffsets[i] = glm::vec2(i, i);
    }

    return squad;
}