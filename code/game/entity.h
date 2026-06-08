#ifndef ENTITY_H

#include "mesh.h"
#include "model.h"

#include <glm/vec3.hpp>

struct Game;

enum EntityType
{
    EntityType_Static = 1,
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
    u16 id;
    u16 type;
    char textId[25];
    char modelName[64];

    Model *model;
    glm::mat4 *nodeTransforms;
    TransformOverride turret;
    TransformOverride gun;
    TransformOverride gunTip;

    glm::mat4 *skinningMatrices;
    int numOfMatrices;
    int currentAnimation;
    float time;

    bool snapToTerrain = false;
    bool isSelectable = true;

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