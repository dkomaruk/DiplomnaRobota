#ifndef PARTICLE_SYSTEM_H

#include "defines.h"

#include "texture.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>

struct Game;

struct Particle
{
    glm::vec3 pos;
    glm::vec3 velocity;

    float rotation;
    float rotationVelocity;

    float scale;
    float scaleVelocity;

    float startingAlpha;
    glm::vec4 color;
    glm::vec4 colorOut;
    glm::vec4 colorVelocity;
};

//Having a separate buffer with these structs to pass to the GPU is convenient,
//because I can put alive sorted particles in and not worry about order in the main Particle array
struct ParticleData
{
    float scale;
    float angle;
    glm::vec3 offset;
    glm::vec4 color;

    //TODO: cameraDist is getting sent to the GPU I think, because of sizeof(ParticleData) in glBufferSubData
    float cameraDist; //Wasted bandwidth to the GPU
};

struct ParticleSystem
{
    Particle *particles;

    int maxNumOfParticles;
    uint32 nextParticle;

    int spawnRate = 13;
    float accumulatedSpawns;
    int aliveParticles;

    float radius = 1.0f;

    float minScale = 1.0f;
    float maxScale = 1.0f;
    float minScaleVelocity = 0.0f;
    float maxScaleVelocity = 0.0f;

    glm::vec3 minPos = glm::vec3(0.0f, 0.0f, 3.5f);
    glm::vec3 maxPos = glm::vec3(0.0f, 0.0f, 3.5f);
    glm::vec3 minVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 maxVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec4 minColor = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    glm::vec4 maxColor = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    glm::vec4 minColorVelocity = glm::vec4(0.0f, 0.0f, 0.0f, -0.083f);
    glm::vec4 maxColorVelocity = glm::vec4(0.0f, 0.0f, 0.0f, -0.083f);
};

int CompareParticles(const void *a, const void *b);

ParticleSystem InitParticleSystem(Game *game);
void SpawnParticles(Game *game, ParticleSystem *system);
void UpdateParticles(Game *game, ParticleSystem *system);
void SortAllParticles(Game *game);
void RenderParticles(Game *game);

#define PARTICLE_SYSTEM_H
#endif