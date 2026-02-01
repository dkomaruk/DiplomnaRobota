#ifndef ENTITY_H

#include <glm/vec3.hpp>

struct Game;

struct Mesh;
struct Model;

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
    char textId[25];

    Model *models;
    int numOfModels;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale = glm::vec3(1.0f);

    RenderEntityFunc *Render;
};

Entity CreateEntity(Mesh *meshes, int numOfMeshes=1);
Entity CreateEntity(Model *model);

#define ENTITY_H
#endif