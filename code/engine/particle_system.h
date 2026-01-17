#ifndef PARTICLE_SYSTEM_H

#include "defines.h"

#include "texture.h"
#include "timer.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>

#include <expat.h>

#include <imgui.h>
#include <imgui_gradient/imgui_gradient.hpp>

#define PARTICLES_MAX_CONTROL_POINTS 10

struct Game;

struct Particle
{
    float timeLeft;

    glm::vec2 uvOffset;
    glm::vec2 uvScale;

    glm::vec3 pos;
    glm::vec3 velocity;
    glm::vec3 initialVelocity;
    glm::vec3 acceleration;

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
    glm::vec2 uvOffset;
    glm::vec2 uvScale;
    glm::vec3 offset;
    glm::vec4 color;

    //TODO: cameraDist is getting sent to the GPU I think, because of sizeof(ParticleData) in glBufferSubData
    float cameraDist; //Wasted bandwidth to the GPU
};

struct ParticleSystemSettings
{
    int maxNumOfParticles = 300;

    bool prewarm = false;
    float prewarmSeconds = 5.0f;

    int spawnRate = 13;
    float lifetime = 10.0f;
    bool limitedLife = true;

    float radius = 1.0f;

    float minRotation = 0.0f;
    float maxRotation = 360.0f;
    float minRotationSpeed = 0.0f;
    float maxRotationSpeed = 20.0f;

    float minScale = 1.0f;
    float maxScale = 1.0f;
    float minScaleVelocity = 0.0f;
    float maxScaleVelocity = 0.0f;

    glm::vec3 minOffset = glm::vec3(0.0f, 0.0f, 3.5f);
    glm::vec3 maxOffset = glm::vec3(0.0f, 0.0f, 3.5f);
    glm::vec3 minVelocity = glm::vec3(0.0f);
    glm::vec3 maxVelocity = glm::vec3(0.0f);

    glm::vec3 minAccel = glm::vec3(0.0f);
    glm::vec3 maxAccel = glm::vec3(0.0f);

    glm::vec4 minColor = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    glm::vec4 maxColor = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
    glm::vec4 minColorVelocity = glm::vec4(0.0f, 0.0f, 0.0f, -0.083f);
    glm::vec4 maxColorVelocity = glm::vec4(0.0f, 0.0f, 0.0f, -0.083f);

    bool isAnimated = false;
    int animationFPS = 120;
    Atlas *atlas;

    union
    {
        //Max size + 1, because otherwise ImGui::Curve overflows buffer
        glm::vec2 velocityControlPoints[PARTICLES_MAX_CONTROL_POINTS + 1];
        ImVec2 imVelocityControlPoints[PARTICLES_MAX_CONTROL_POINTS + 1] = {ImVec2(ImGui::CurveTerminator, 0.0f)};
    };
    bool velocityOverLifetime = false;
    bool colorOverLifetime = false;

    ImGG::GradientWidget gradientWgt;
};

struct ParticleSystem
{
    Particle *particles;
    uint32 nextParticle;
    float accumulatedSpawns;
    int aliveParticles;

    Timer prewarmTimer;

    glm::vec3 pos;

    ParticleSystemSettings *settings;
};

int CompareParticles(const void *a, const void *b);

ParticleSystem InitParticleSystem(Game *game, ParticleSystemSettings *settings, glm::vec3 *position = NULL);
void SpawnParticles(Game *game, ParticleSystem *system);
void UpdateParticles(Game *game, ParticleSystem *system);
void SortAllParticles(Game *game);
void RenderParticles(Game *game);

ParticleSystemSettings *GetDefaultSettings();

#define PARTICLE_SYSTEM_H
#endif