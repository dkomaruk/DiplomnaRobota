#include "entity.h"

#include "game.h"

#include "graphics/shader.h"
#include "graphics/texture.h"

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

void RenderEntity(Entity *self, Game *game)
{
    if(self->mesh)
    {
        RenderMesh(game, self->mesh, PrepareModelMatrix(self->position, self->rotation, self->scale));
    }
}

Entity CreateEntity(Mesh *mesh)
{
    Entity entity = {};
    entity.mesh = mesh;
    entity.Render = RenderEntity;
    entity.type = EntityType_Static;

    return entity;
}
