#ifndef ENTITY_H

#include "engine.h"

#include "graphics/mesh.h"

#include <glm/vec3.hpp>

struct Entity
{
    Mesh mesh;

    glm::vec3 position, rotation;
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 color;
};

Entity CreateEntity(Mesh mesh, glm::vec3 color = glm::vec3(0.0f));
void RenderEntity(Engine *engine, Entity *entity);

#define ENTITY_H
#endif