#include "particle_system.h"

#include "mesh.h"
#include "shader.h"

#include "game.h"

#include <glm/gtx/norm.hpp>

#include "math_utils.h"
#include "random.h"

int CompareParticles(const void *a, const void *b)
{
    ParticleData *p1 = (ParticleData *)a;
    ParticleData *p2 = (ParticleData *)b;

    if (p1->cameraDist > p2->cameraDist) return -1;
    if (p1->cameraDist < p2->cameraDist) return 1;

    return 0;
}

ParticleSystem InitParticleSystem(Game *game, ParticleSystemSettings *settings, glm::vec3 *position)
{
    ParticleSystem system = {};
    system.settings = settings;

    system.particles = (Particle *)calloc(settings->maxNumOfParticles, sizeof(Particle));

    if(!position)
    {
        system.pos = glm::vec3(RandomBetween(-20.0f, 20.0f), 0.0f, RandomBetween(-20.0f, 20.0f));
    }

    return system;
}

void SpawnParticles(Game *game, ParticleSystem *system)
{
    ParticleSystemSettings *settings = system->settings;

    int deadParticles = settings->maxNumOfParticles - system->aliveParticles;
    int particlesToSpawn = (int)system->accumulatedSpawns;
    particlesToSpawn = Min(deadParticles, particlesToSpawn);

    system->accumulatedSpawns += settings->spawnRate * game->deltaTime;
    system->accumulatedSpawns -= particlesToSpawn;

    for(int i = 0; i < particlesToSpawn; ++i)
    {
        Particle *particle = system->particles + system->nextParticle++;

        if((int)system->nextParticle >= settings->maxNumOfParticles)
        {
            system->nextParticle = 0;
        }

        particle->timeLeft = settings->lifetime;

        glm::vec3 dir;
        do
        {
            dir = glm::vec3(RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f));
        } while(glm::length(dir) > 1.0f);

        dir = glm::normalize(dir);
        float radius = RandomBetween(0.0f, settings->radius);
        radius = cbrtf(radius);
        float spawnRadius = 0.5f * 2;
        particle->pos = system->pos + RandomBetween(settings->minOffset, settings->maxOffset) + dir * radius * spawnRadius;

        particle->scale = RandomBetween(settings->minScale, settings->maxScale);
        particle->scaleVelocity = RandomBetween(settings->minScaleVelocity, settings->maxScaleVelocity);

        particle->initialVelocity = RandomBetween(settings->minVelocity, settings->maxVelocity);
        particle->velocity = particle->initialVelocity;
        particle->acceleration = RandomBetween(settings->minAccel, settings->maxAccel);

        particle->rotation = glm::radians(RandomBetween(0.0f, 360.0f));
        particle->rotationVelocity = glm::radians(RandomBetween(0.0f, 20.0f));

        particle->color = glm::vec4(RandomBetween(settings->minColor.r, settings->maxColor.r),
                                    RandomBetween(settings->minColor.g, settings->maxColor.g),
                                    RandomBetween(settings->minColor.b, settings->maxColor.b),
                                    RandomBetween(settings->minColor.a, settings->maxColor.a));
        particle->startingAlpha = particle->color.a;
        particle->colorVelocity = RandomBetween(settings->minColorVelocity, settings->maxColorVelocity);
    }
}

void UpdateParticles(Game *game, ParticleSystem *system)
{
    int maxNumOfParticles = system->settings->maxNumOfParticles;
    bool limitedLife = system->settings->limitedLife;

    UpdateTimer(&system->prewarmTimer, game->deltaTime);

    for(int i = 0; i < maxNumOfParticles; ++i)
    {
        Particle *particle = system->particles + i;

        if((particle->color.a > 0.0f) && ((particle->timeLeft > 0.0f) || !limitedLife))
        {
            float deltaTime = (limitedLife && (particle->timeLeft - game->deltaTime < 0.0f)) ? particle->timeLeft
                                                                                             : game->deltaTime;
            particle->timeLeft -= deltaTime;

            if(system->settings->velocityOverLifetime && limitedLife)
            {
                float p = 1.0f - (particle->timeLeft / system->settings->lifetime);
                particle->velocity = (ImGui::CurveValueSmooth(p, PARTICLES_MAX_CONTROL_POINTS, system->settings->imVelocityControlPoints)) * particle->initialVelocity;
            }
            else
            {
                particle->velocity += particle->acceleration * deltaTime;
            }

            particle->pos += (0.5f * particle->acceleration * Square(deltaTime)) +
                             (particle->velocity * deltaTime);

            particle->scale += particle->scaleVelocity * deltaTime;
            particle->rotation += particle->rotationVelocity * deltaTime;

            if((particle->color.a + (particle->colorVelocity.a * deltaTime)) < 0.0f)
            {
                particle->colorVelocity.a *= 0.01f;
            }

            particle->color += particle->colorVelocity * deltaTime;
            particle->colorOut = particle->color;
            particle->colorOut.a = Max(0.0f, particle->color.a);

            //Slowly fade in after beign spawned instead of popping into existence immediately
            float fadeInFactor = 0.1f;
            float alphaThreshold = particle->startingAlpha - fadeInFactor;
            if(particle->colorOut.a > alphaThreshold)
            {
                particle->colorOut.a = alphaThreshold * Clamp01MapToRange(particle->startingAlpha, particle->colorOut.a, alphaThreshold);
            }
        }
    }
}

void SortAllParticles(Game *game)
{
    glm::vec3 cameraPos = game->camera.position;

    game->aliveParticles = 0;
    int deadParticles = 0;

    for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
    {
        ParticleSystem *system = &game->particleSystems[i];
        int maxNumOfParticles = system->settings->maxNumOfParticles;
        bool limitedLife = system->settings->limitedLife;

        int aliveParticles = 0;
        if(system->prewarmTimer.isFinished)
        {
            for(int j = 0; j < maxNumOfParticles; ++j)
            {
                Particle *particle = system->particles + j;

                if((particle->color.a > 0.0f) && ((particle->timeLeft > 0.0f) || !limitedLife))
                {
                    game->particleData[game->aliveParticles].scale = particle->scale;
                    game->particleData[game->aliveParticles].angle = particle->rotation;
                    game->particleData[game->aliveParticles].offset = particle->pos;
                    game->particleData[game->aliveParticles].color = particle->colorOut;

                    glm::vec3 diff = cameraPos - particle->pos;
                    game->particleData[game->aliveParticles].cameraDist = glm::length2(diff);

                    ++game->aliveParticles;
                    ++aliveParticles;
                }
            }
        }

        deadParticles += maxNumOfParticles - aliveParticles;
        system->aliveParticles = aliveParticles;
    }

    qsort(game->particleData, game->aliveParticles, sizeof(ParticleData), CompareParticles);

    glBindBuffer(GL_ARRAY_BUFFER, game->vboInstances);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * game->aliveParticles, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleData) * game->aliveParticles, game->particleData);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    char buffer[25];
    sprintf(buffer, "Alive Particles: %d", game->aliveParticles);
    UpdateText(&game->aliveParticlesText, buffer);

    sprintf(buffer, "Dead Particles: %d", deadParticles);
    UpdateText(&game->deadParticlesText, buffer);
}

void RenderParticles(Game *game)
{
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    UseShader(game->particleShader);

    ShaderSetInt(game->particleShader, "u_texture", 0);
    SetTexture(game->textureID, 0);

    glBindVertexArray(game->particlesQuad.vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, game->aliveParticles);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}