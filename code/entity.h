#ifndef ENTITY_H

#include "engine.h"

#include "graphics/mesh.h"

#include <glm/vec3.hpp>

struct Entity
{
    Mesh mesh;

    glm::vec3 position, rotation;
    glm::vec3 scale = glm::vec3(1.0f);
};

Entity CreateEntity(Mesh mesh);
void RenderEntity(Engine *engine, Entity *entity);

#define ENTITY_H
#endif