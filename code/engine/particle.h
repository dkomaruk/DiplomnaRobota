#ifndef PARTICLE_H

#include "texture.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>

struct Particle
{
    glm::vec3 pos;
    glm::vec3 velocity;

    float rotation;
    float rotationVelocity;

    glm::vec4 color;
    glm::vec4 colorVelocity;
};

glm::mat4 UpdateParticleMatrix(Particle *particle, Texture *texture, float scale = 1.0f);

#define PARTICLE_H
#endif