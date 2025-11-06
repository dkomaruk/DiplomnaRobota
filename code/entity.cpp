#include "entity.h"

#include "game.h"

#include "graphics/shader.h"
#include "graphics/texture.h"

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void RenderEntity(Entity *self, Game *game)
{
    for(int i = 0; i < self->model.numOfMeshes; i++)
    {
        RenderMesh(game, &self->model.meshes[i], PrepareModelMatrix(self->position, self->rotation, self->scale));
    }
}

Entity CreateEntity(Mesh *meshes, int numOfMeshes)
{
    Entity entity = {};

    entity.model.numOfMeshes = numOfMeshes;
    entity.model.meshes = meshes;

    entity.Render = RenderEntity;
    entity.type = EntityType_Static;

    return entity;
}

Entity CreateEntity(Model *model)
{
    return CreateEntity(model->meshes, model->numOfMeshes);
}
