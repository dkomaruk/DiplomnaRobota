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

#if 0
    if(game->pickingPass)
    {
        ShaderSetUInt(game->pickingShader, "u_objectIndex", self->id);
        ShaderSetUInt(game->skinnedPickingShader, "u_objectIndex", self->id);
    }
#endif

    if(game->renderAABB)
    {
        ShaderSetVec3(game->lineShader, "u_color", glm::vec3(0.0f, 1.0f, 0.0f));
        glLineWidth(2.0f);
        RenderMesh(game, &self->meshAABB, self->modelMatPosScale, game->lineShader, 0, self->meshAABB.drawMode);
        glLineWidth(1.0f);
    }

    RenderModel(game, self->model, self->modelMat, self->nodeTransforms, self->skinningMatrices);
}

Entity CreateEntity(Model *model)
{
    Entity entity = {};

    entity.model = model;
    //entity.numOfModels = 1;
    entity.Render = RenderEntity;
    entity.type = EntityType_Static;

    entity.aabb = model->aabb;
    uint32 indicesAABB[24] = {
        0, 1, 1, 3, 3, 2, 2, 0,
        4, 5, 5, 7, 7, 6, 6, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    };

    AttribInfo attrib = {0, 3, GL_FLOAT, sizeof(glm::vec3), (void *)0};
    entity.meshAABB = CreateMesh(&entity.aabb.corners[0], 8, sizeof(glm::vec3),
                                  &indicesAABB[0], 24, &attrib, 1, GL_DYNAMIC_DRAW);
    entity.meshAABB.drawMode = GL_LINES;

    //entity.localTransforms = (glm::mat4 *)calloc(model->numOfNodes, sizeof(glm::mat4));
    //for(int nodeIndex = 0; nodeIndex < model->numOfNodes; nodeIndex++)
    //{
        //entity.localTransforms[nodeIndex] = model->nodes[nodeIndex].localTransform;
    //}

    entity.nodeTransforms = (glm::mat4 *)calloc(model->numOfNodes, sizeof(glm::mat4));
    UpdateTransforms(&entity);

    if(model->type == ModelType_Animated)
    {
        entity.numOfMatrices = model->animData->skeleton.numOfBones;
        entity.skinningMatrices = (glm::mat4 *)calloc(entity.numOfMatrices, sizeof(glm::mat4));

        for(int i = 0; i < entity.numOfMatrices; i++)
        {
            entity.skinningMatrices[i] = glm::mat4(1.0f);
        }
    }

    return entity;
}

void DeleteEntity(Entity *entity)
{
    free(entity->nodeTransforms);
    free(entity);
}

void UpdateTransforms(Entity *entity)
{
    for (int nodeIndex = 0; nodeIndex < entity->model->numOfNodes; ++nodeIndex) {
        Node *node = &entity->model->nodes[nodeIndex];
        //glm::mat4 nodeTransform = entity->localTransforms[nodeIndex];

        glm::mat4 nodeTransform;
        if(entity->turret.nodeId == nodeIndex)
            nodeTransform = entity->turret.transform;
        else if(entity->gun.nodeId == nodeIndex)
            nodeTransform = entity->gun.transform;
        else
            nodeTransform = entity->model->nodes[nodeIndex].localTransform;

        if(node->parentId != -1)
        {
            nodeTransform = entity->nodeTransforms[node->parentId] * nodeTransform;
        }
        entity->nodeTransforms[nodeIndex] = nodeTransform;
    }
}

void UpdateEntity(Game *game, Entity *entity)
{
    if(entity->model && (entity->model->type == ModelType_Animated))
    {
        UpdateAnimation(entity, game->deltaTime);
    }

    entity->modelMatPosScale = PrepareModelMatrix(entity->position, glm::vec3(0.0f), entity->scale);
    entity->modelMat = PrepareModelMatrix(entity);
}

glm::mat4 PrepareModelMatrix(Entity *entity)
{
    return PrepareModelMatrix(entity->position, entity->rotation, entity->scale);
}