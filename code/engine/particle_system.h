#ifndef PARTICLE_SYSTEM_H

#include "defines.h"

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
    glm::vec4 colorOut;
    glm::vec4 colorVelocity;
};

struct ParticleData
{
    float angle;
    glm::vec3 offset;
    glm::vec4 color;

    float cameraDist;
};

struct ParticleSystem
{
    Particle *particles;
    ParticleData *particleData;
    int numOfParticles;

    int spawnRate = 13;

    uint32 nextParticle;

    float radius = 1.0f;

    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.5f);

    //glm::vec4 minColor = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);
    glm::vec4 minColor = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    glm::vec4 maxColor = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    glm::vec4 colorVelocity = glm::vec4(0.0f, 0.0f, 0.0f, -0.083f);
};

int CompareParticles(const void *a, const void *b);

#define PARTICLE_SYSTEM_H
#endif