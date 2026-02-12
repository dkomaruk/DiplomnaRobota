#ifndef ENTITY_H

#include "mesh.h"
#include "model.h"

#include <glm/vec3.hpp>

struct Game;

enum EntityType
{
    EntityType_Static = 1,
    EntityType_Infantry
};

struct Entity;

typedef void RenderEntityFunc(Entity *self, Game *game);

struct TransformOverride
{
    int nodeId = -1;
    glm::mat4 transform;
};

struct Entity
{
    uint16 id;
    uint16 type;
    char textId[25];

    Model *model;
    //int numOfModels;
    //glm::mat4 *localTransforms;
    glm::mat4 *nodeTransforms;
    TransformOverride turret;
    TransformOverride gun;
    //int turretId;
    //int gunTipId;

    AABB aabb;
    Mesh meshAABB;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 modelMat;
    glm::mat4 modelMatPosScale;

    RenderEntityFunc *Render;
};

Entity CreateEntity(Model *model);
void DeleteEntity(Entity *entity);

void UpdateEntity(Game *game, Entity *entity);
void UpdateTransforms(Entity *entity);

glm::mat4 PrepareModelMatrix(Entity *entity);

#define ENTITY_H
#endif