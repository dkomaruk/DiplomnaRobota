#include "entity.h"

#include "game.h"

#include "shader.h"
#include "texture.h"

#include "mesh.h"
#include "model.h"

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void RenderEntity(Entity *self, Game *game)
{
    if(!self->models || !self->numOfModels) return;

    if(game->pickingPass)
    {
        ShaderSetUInt(game->pickingShader, "u_objectIndex", self->id);
        ShaderSetUInt(game->skinnedPickingShader, "u_objectIndex", self->id);
    }
    RenderModel(game, self->models, PrepareModelMatrix(self->position, self->rotation, self->scale));
}

Entity CreateEntity(Mesh *meshes, int numOfMeshes)
{
    Entity entity = {};

    entity.models = (Model *)malloc(sizeof(Model));
    entity.models->numOfMeshes = numOfMeshes;
    entity.models->mesh = meshes;
    entity.numOfModels = 1;

    entity.Render = RenderEntity;
    entity.type = EntityType_Static;

    return entity;
}

Entity CreateEntity(Model *model)
{
    Entity entity = {};
    entity.models = model;
    entity.numOfModels = 1;
    entity.Render = RenderEntity;
    entity.type = EntityType_Static;
    return entity;
}
