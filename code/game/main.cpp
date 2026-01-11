#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp> //Has to be included before any other glm header file

#include "stb_image.cpp"
#include "stb_image_write.cpp"

#include "input.cpp"

#include "infantry.cpp"
#include "entity.cpp"
#include "game.cpp"

#include "audio.cpp"
#include "asset_loader.cpp"

#include "timer.cpp"
#include "text.cpp"
#include "text_demo.cpp"

#include "light.cpp"
#include "camera.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "image.cpp"
#include "mesh.cpp"
#include "particle_system.cpp"
#include "editor_ui.cpp"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <random>
#include <time.h>

#include <imgui.h>

float RandomBetween(float min, float max)
{
    if(min == max) return max;
    return min + (((float)rand() / RAND_MAX) * (max - min));
}

inline float Clamp(float min, float value, float max)
{
    float result = value;

    if(result < min)
    {
        result = min;
    }
    else if(result > max)
    {
        result = max;
    }

    return(result);
}

inline float Clamp01(float value)
{
    float result = Clamp(0.0f, value, 1.0f);
    return(result);
}

inline float Clamp01MapToRange(float min, float t, float max)
{
    float result = 0.0f;

    float range = max - min;
    if(range != 0.0f)
    {
        result = Clamp01((t - min) / range);
    }

    return(result);
}

int main(int argc, char *argv[])
{
    srand((uint32)time(0));

    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    LoadAssets(game);

    game->lastFrame = SDL_GetPerformanceCounter();
    //game->lockFPS = true;

    Texture particleTexture = CreateTexture("../data/imgs/smoke2.png");

    ParticleSystem *smoke = &game->smokeParticles;
    smoke->numOfParticles = 300;
    smoke->particles = (Particle *)calloc(smoke->numOfParticles, sizeof(Particle));
    smoke->particleData = (ParticleData *)calloc(smoke->numOfParticles, sizeof(ParticleData));

    Mesh quad = CreateUnitQuadStripes();

    glBindVertexArray(quad.vao);

    GLuint vboInstances;
    glGenBuffers(1, &vboInstances);
    glBindBuffer(GL_ARRAY_BUFFER, vboInstances);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * smoke->numOfParticles, smoke->particleData, GL_STREAM_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)(0));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)(1 * sizeof(float)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)(1 * sizeof(glm::vec3) + 1 * sizeof(float)));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    bool renderParticles = true;

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateEditorUI(game);
        UpdateGame(game);

        static float a = 0;
        a += smoke->spawnRate * game->deltaTime;
        int particlesToSpawn = (int)a;
        a -= particlesToSpawn;

        for(int i = 0; i < particlesToSpawn; ++i)
        {
            Particle *particle = smoke->particles + smoke->nextParticle++;

            if((int)smoke->nextParticle >= smoke->numOfParticles)
            {
                smoke->nextParticle = 0;
            }

            glm::vec3 dir;
            do
            {
                dir = glm::vec3( RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f), RandomBetween(-1.0f, 1.0f));
            } while(glm::length(dir) > 1.0f);

            dir = glm::normalize(dir);
            float radius = RandomBetween(0.0f, smoke->radius);
            radius = cbrtf(radius);
            float spawnRadius = 0.5f * 2;
            particle->pos = smoke->pos + dir * radius * spawnRadius;

            //particle->velocity = glm::vec3(RandomBetween(-0.2f, 0.2f), RandomBetween(0.1f, 0.05f) * 2.0f, 0.0f);
            //particle->velocity = glm::normalize(particle->velocity) * 70.0f;

            particle->rotation = glm::radians(RandomBetween(0.0f, 360.0f));
            particle->rotationVelocity = glm::radians(RandomBetween(0.0f, 20.0f));

            particle->color = glm::vec4(RandomBetween(smoke->minColor.r, smoke->maxColor.r),
                                        RandomBetween(smoke->minColor.g, smoke->maxColor.g),
                                        RandomBetween(smoke->minColor.b, smoke->maxColor.b),
                                        RandomBetween(smoke->minColor.a, smoke->maxColor.a));

            particle->colorVelocity = smoke->colorVelocity;
        }

        for(int i = 0; i < smoke->numOfParticles; ++i)
        {
            Particle *particle = smoke->particles + i;

            if(particle->velocity != glm::vec3(0.0f))
            {
                SDL_Log("ALARM IDIOT");
            }

            if(particle->color.a > 0.0f)
            {
                particle->pos += particle->velocity * game->deltaTime;
                particle->rotation += particle->rotationVelocity * game->deltaTime;
                particle->color += particle->colorVelocity * game->deltaTime;

                particle->colorOut = particle->color;
                //particle->colorOut.y = Clamp01(particle->color.y);
                particle->colorOut.a = Clamp01(particle->color.a);

                float alphaThreshold = smoke->maxColor.a - 0.1f;
                if(particle->colorOut.a > alphaThreshold)
                {
                    particle->colorOut.a = alphaThreshold * Clamp01MapToRange(smoke->maxColor.a, particle->colorOut.a, alphaThreshold);
                }

            }
        }

        if(IsFirstPress(game, SDL_SCANCODE_Y))
        {
            renderParticles = !renderParticles;
        }

        int aliveParticles = 0;
        if(renderParticles)
        {
            uint64 start = SDL_GetPerformanceCounter();

            glm::vec3 cameraPos = game->camera.position;
            for(int i = 0; i < smoke->numOfParticles; i++)
            {
                Particle *particle = smoke->particles + i;

                if(particle->color.a > 0.0f)
                {
                    smoke->particleData[aliveParticles].angle = particle->rotation;
                    smoke->particleData[aliveParticles].offset = particle->pos;
                    smoke->particleData[aliveParticles].color = particle->colorOut;

                    glm::vec3 diff = cameraPos - particle->pos;
                    smoke->particleData[aliveParticles].cameraDist = glm::length2(diff);

                    ++aliveParticles;
                }
            }

            qsort(smoke->particleData, aliveParticles, sizeof(ParticleData), CompareParticles);

            uint64 end = SDL_GetPerformanceCounter();
            SDL_Log("%f ms", (float)(end - start) / game->perfFreq * 1000.0f);

            glBindBuffer(GL_ARRAY_BUFFER, vboInstances);
            glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * aliveParticles, NULL, GL_STREAM_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ParticleData) * aliveParticles, smoke->particleData);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        char dynamicBuffer[20];
        sprintf(dynamicBuffer, "Alive Particles: %d", aliveParticles);
        UpdateText(&game->aliveParticles, dynamicBuffer);

        //Rendering
        if(!game->textDemoEnabled)
        {
            glEnable(GL_DEPTH_TEST);

            game->pickingPass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo);
            RenderScene(game);
            game->pickingPass = false;

            if(IsFirstClick(game, MOUSE_LEFT))
            {
                float x, y;
                if(game->isCursorHidden)
                {
                    x = WINDOW_WIDTH / 2.0f;
                    y = WINDOW_HEIGHT / 2.0f;
                }
                else
                {
                    SDL_GetMouseState(&x, &y);
                    y = (int)WINDOW_HEIGHT - y;
                }

                uint8 pixels[3];
                glReadPixels((int)x, (int)y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
                uint32 pickedID = pixels[0];

                if(!pickedID || !game->keys[SDL_SCANCODE_LSHIFT])
                {
                    game->selectedIDs.clear();
                }

                bool isAlreadyPicked = game->selectedIDs.count(pickedID);
                if(isAlreadyPicked && game->keys[SDL_SCANCODE_LSHIFT])
                {
                    game->selectedIDs.erase(pickedID);
                }
                else
                {
                    game->selectedIDs.insert(pickedID);
                }
            }

            game->outlinePass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineTexture.id, 0);
            RenderScene(game);
            game->outlinePass = false;

            glDepthMask(GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
            RenderScene(game);

            if(renderParticles)
            {
                glEnable(GL_BLEND);
                //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDepthMask(GL_FALSE);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);

                UseShader(game->particleShader);

                ShaderSetInt(game->particleShader, "u_texture", 0);
                SetTexture(&particleTexture, 0);

                glBindVertexArray(quad.vao);
                //glDrawArraysInstanced(GL_TRIANGLES, 0, quad.verticesCount, smoke->numOfParticles);
                glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, aliveParticles);
                glBindVertexArray(0);

                glDisable(GL_BLEND);
                glDisable(GL_CULL_FACE);
                glDepthMask(GL_TRUE);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            SetTexture(&game->outlineTexture, 0);
            ShaderSetInt(game->postProcessShader, "u_outline", 0);
            SetTexture(&game->fullSceneTexture, 1);
            ShaderSetInt(game->postProcessShader, "u_scene", 1);

            glBindVertexArray(game->fullscreenQuad.vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            RenderText(&game->aliveParticles);
            RenderText(&game->fpsCounter);
            RenderText(&game->msPerFrame);

            glDisable(GL_BLEND);
        }
        else
        {
            RenderTextDemo(game);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(game->window);

        Uint64 thisFrame = SDL_GetPerformanceCounter();
        if(game->lockFPS)
        {
            int targetFrames = 20;
            float targetTime = 1.0f / targetFrames;
            float elapsedWhileWaiting = 0.0f;
            while((elapsedWhileWaiting = (SDL_GetPerformanceCounter() - thisFrame) / (float)game->perfFreq) < targetTime)
            {
                SDL_Delay((uint32)((targetTime - elapsedWhileWaiting) * 1000));
            }

            thisFrame = SDL_GetPerformanceCounter();
        }

        game->deltaTime = (thisFrame - game->lastFrame) / (float)game->perfFreq;

        float ms = game->deltaTime * 1000.0f;
        float fps = 1000.0f / ms;
        //SDL_Log("%f", fps);

        sprintf(dynamicBuffer, "%.5f FPS", fps);

        UpdateText(&game->fpsCounter, dynamicBuffer);

        sprintf(dynamicBuffer, "%.5f ms/f", ms);
        UpdateText(&game->msPerFrame, dynamicBuffer);

        game->lastFrame = thisFrame;
    }

    SDL_Log("Exited the main loop\n");


    return 0;
}

