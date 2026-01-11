#include "particle_system.h"

#include "mesh.h"
#include "shader.h"

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

ParticleSystem InitParticleSystem(Game *game)
{
    ParticleSystem system = {};
    system.maxNumOfParticles = 300;
    system.particles = (Particle *)calloc(system.maxNumOfParticles, sizeof(Particle));
    system.pos = glm::vec3(RandomBetween(-20.0f, 20.0f), 0.0f, RandomBetween(-20.0f, 20.0f));

    return system;
}

void SpawnParticles(Game *game, ParticleSystem *system)
{
    system->accumulatedSpawns += system->spawnRate * game->deltaTime;
    int particlesToSpawn = (int)system->accumulatedSpawns;
    system->accumulatedSpawns -= particlesToSpawn;

    for(int i = 0; i < particlesToSpawn; ++i)
    {
        Particle *particle = system->particles + system->nextParticle++;

        if((int)system->nextParticle >= system->maxNumOfParticles)
        {
            system->nextParticle = 0;
        }

        glm::vec3 dir;
        do
        {
            dir = glm::vec3(RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f));
        } while(glm::length(dir) > 1.0f);

        dir = glm::normalize(dir);
        float radius = RandomBetween(0.0f, system->radius);
        radius = cbrtf(radius);
        float spawnRadius = 0.5f * 2;
        particle->pos = system->pos + dir * radius * spawnRadius;

        //particle->velocity = glm::vec3(RandomBetween(-0.2f, 0.2f), RandomBetween(0.1f, 0.05f) * 2.0f, 0.0f);
        //particle->velocity = glm::normalize(particle->velocity) * 70.0f;

        particle->rotation = glm::radians(RandomBetween(0.0f, 360.0f));
        particle->rotationVelocity = glm::radians(RandomBetween(0.0f, 20.0f));

        particle->color = glm::vec4(RandomBetween(system->minColor.r, system->maxColor.r),
                                    RandomBetween(system->minColor.g, system->maxColor.g),
                                    RandomBetween(system->minColor.b, system->maxColor.b),
                                    RandomBetween(system->minColor.a, system->maxColor.a));

        particle->colorVelocity = system->colorVelocity;
    }
}

void UpdateParticles(Game *game, ParticleSystem *system)
{
    for(int i = 0; i < system->maxNumOfParticles; ++i)
    {
        Particle *particle = system->particles + i;

        if(particle->color.a > 0.0f)
        {
            particle->pos += particle->velocity * game->deltaTime;
            particle->rotation += particle->rotationVelocity * game->deltaTime;
            particle->color += particle->colorVelocity * game->deltaTime;

            particle->colorOut = particle->color;
            //particle->colorOut.y = Clamp01(particle->color.y);
            particle->colorOut.a = Clamp01(particle->color.a);

            //Slowly fade in instead of popping into existence immediately
            float alphaThreshold = system->maxColor.a - 0.1f;
            if(particle->colorOut.a > alphaThreshold)
            {
                particle->colorOut.a = alphaThreshold * Clamp01MapToRange(system->maxColor.a, particle->colorOut.a, alphaThreshold);
            }
        }
    }
}

void SortAllParticles(Game *game)
{
    glm::vec3 cameraPos = game->camera.position;

    game->aliveParticles = 0;
    for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
    {
        ParticleSystem *system = &game->particleSystems[i];

        for(int j = 0; j < system->maxNumOfParticles; ++j)
        {
            Particle *particle = system->particles + j;

            if(particle->color.a > 0.0f)
            {
                game->particleData[game->aliveParticles].angle = particle->rotation;
                game->particleData[game->aliveParticles].offset = particle->pos;
                game->particleData[game->aliveParticles].color = particle->colorOut;

                glm::vec3 diff = cameraPos - particle->pos;
                game->particleData[game->aliveParticles].cameraDist = glm::length2(diff);

                ++game->aliveParticles;
            }
        }
    }

    qsort(game->particleData, game->aliveParticles, sizeof(ParticleData), CompareParticles);

    glBindBuffer(GL_ARRAY_BUFFER, game->vboInstances);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * game->aliveParticles, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleData) * game->aliveParticles, game->particleData);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    char buffer[20];
    sprintf(buffer, "Alive Particles: %d", game->aliveParticles);
    UpdateText(&game->aliveParticlesText, buffer);
}

void RenderParticles(Game *game)
{
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    UseShader(game->particleShader);

    ShaderSetInt(game->particleShader, "u_texture", 0);
    SetTexture(game->textureID, 0);

    glBindVertexArray(game->particlesQuad.vao);
    //glDrawArraysInstanced(GL_TRIANGLES, 0, quad.verticesCount, smoke->numOfParticles);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, game->aliveParticles);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}