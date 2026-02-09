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
    if(!self->model /*|| !self->numOfModels*/) return;

    if(game->pickingPass)
    {
        ShaderSetUInt(game->pickingShader, "u_objectIndex", self->id);
        ShaderSetUInt(game->skinnedPickingShader, "u_objectIndex", self->id);
    }
    RenderModel(game, self->model, PrepareModelMatrix(self->position, self->rotation, self->scale), self->nodeTransforms);
}

Entity CreateEntity(Model *model)
{
    Entity entity = {};

    entity.model = model;
    //entity.numOfModels = 1;
    entity.Render = RenderEntity;
    entity.type = EntityType_Static;

    //entity.localTransforms = (glm::mat4 *)calloc(model->numOfNodes, sizeof(glm::mat4));
    //for(int nodeIndex = 0; nodeIndex < model->numOfNodes; nodeIndex++)
    //{
        //entity.localTransforms[nodeIndex] = model->nodes[nodeIndex].localTransform;
    //}

    entity.nodeTransforms = (glm::mat4 *)calloc(model->numOfNodes, sizeof(glm::mat4));
    UpdateTransforms(&entity);

    return entity;
}

void UpdateTransforms(Entity *entity)
{
    for (int nodeIndex = 0; nodeIndex < entity->model->numOfNodes; ++nodeIndex) {
        Node *node = &entity->model->nodes[nodeIndex];
        //glm::mat4 nodeTransform = entity->localTransforms[nodeIndex];


        glm::mat4 nodeTransform = (entity->turret.nodeId == nodeIndex) ? entity->turret.transform
                                                                       : entity->model->nodes[nodeIndex].localTransform;
        if(node->parentId != -1)
        {
            nodeTransform = entity->nodeTransforms[node->parentId] * nodeTransform;
        }
        entity->nodeTransforms[nodeIndex] = nodeTransform;
    }
}

void UpdateEntity(Game *game, Entity *entity)
{
    if(entity->model && entity->model->type == ModelType_Animated)
    {
        UpdateAnimation(entity->model, game->deltaTime);
    }
}