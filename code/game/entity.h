#ifndef ENTITY_H

#include "mesh.h"

#include <glm/vec3.hpp>

struct Game;

enum EntityType
{
    EntityType_Static = 1,
    EntityType_Infantry
};

struct Entity;

typedef void RenderEntityFunc(Entity *self, Game *game);

struct Entity
{
    uint16 id;
    uint16 type;

    Model *models;
    int numOfModels;

    vec3 position;
    vec3 rotation;
    vec3 scale = vec3(1.0f);

    //void (* Render)(Entity *self, Game *game);
    RenderEntityFunc *Render;
};

Entity CreateEntity(Mesh *meshes, int numOfMeshes=1);
Entity CreateEntity(Model *model);

#define ENTITY_H
#endif