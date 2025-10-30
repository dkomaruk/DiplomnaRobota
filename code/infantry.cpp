#include "infantry.h"

#include "util_defines.h"


void RenderInfantry(Entity *e, Game *game)
{
    Assert(e->type == EntityType_Infantry)
    InfantrySquad *squad = (InfantrySquad *)(e);

    Mesh *meshes = squad->meshes;
    for(int i = 0; i < squad->size; i++)
    {
        vec3 offset = vec3(squad->soldierOffsets[i].x, 0.0f, squad->soldierOffsets[i].y);
        mat4 model = PrepareModelMatrix(squad->position + offset, squad->rotation, squad->scale);

        RenderMesh(game, &meshes[squad->soldierMeshesId[i]], model);
    }
}

InfantrySquad CreateInfantrySquad(Mesh *soldierMeshes, int numOfMeshes, int squadSize)
{
    InfantrySquad squad = {};

    squad.type = EntityType_Infantry;
    squad.size = squadSize;
    squad.meshes = soldierMeshes;
    squad.numOfMeshes = numOfMeshes;
    squad.Render = RenderInfantry;

    squad.soldierMeshesId = (int *)malloc(sizeof(int) * squadSize);
    squad.soldierOffsets = (vec2 *)malloc(sizeof(vec2) * squadSize);
    for(int i = 0; i < squadSize; i++)
    {
        squad.soldierMeshesId[i] = i % numOfMeshes;
        squad.soldierOffsets[i] = vec2(i, i);
    }

    return squad;
}