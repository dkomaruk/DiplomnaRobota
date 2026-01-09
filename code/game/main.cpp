
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
#include "particle.cpp"

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
#include <vector>

#include <random>
#include <time.h>

float RandomBetween(float min, float max)
{
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

    Texture particleTexture = CreateTexture("../data/imgs/transparent_smoke_particle.png");

    uint32 nextParticle = 0;
    Particle particles[1024] = {};

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateGame(game);

        for(int i = 0; i < 2; ++i)
        {
            Particle *particle = particles + nextParticle++;

            if(nextParticle == ArrayCount(particles))
            {
                nextParticle = 0;
            }

            float x = (WINDOW_WIDTH / 2.0f) - (particleTexture.x / 2.0f);
            float y = WINDOW_HEIGHT - particleTexture.y * 0.3f;

            particle->pos = glm::vec3(RandomBetween(x - 100.0f, x + 100.0f), RandomBetween(y - 10.0f, y + 10.0f), 0.0f);
            particle->velocity = glm::vec3(RandomBetween(-50.0f, 50.0f), -RandomBetween(40.0f, 80.0f), 0.0f);

            particle->rotation = RandomBetween(0.0f, 360.0f);
            particle->rotationVelocity = RandomBetween(0.0f, 20.0f);

            particle->color = glm::vec4(RandomBetween(0.75f, 1.0f), RandomBetween(0.75f, 1.0f),
                                        RandomBetween(0.75f, 1.0f), 0.3f);

            particle->colorVelocity = glm::vec4(0.0f, 0.0f, 0.0f, RandomBetween(-0.04f, -0.08f));
        }

        for(int i = 0; i < ArrayCount(particles); ++i)
        {
            Particle *particle = particles + i;

            particle->pos += particle->velocity * game->deltaTime;
            particle->rotation += particle->rotationVelocity * game->deltaTime;
            particle->color += particle->colorVelocity * game->deltaTime;

            particle->colorOut = particle->color;
            particle->colorOut.y = Clamp01(particle->colorOut.y);
            particle->colorOut.a = Clamp01(particle->colorOut.a);

            if(particle->colorOut.a > 0.2f)
            {
                particle->colorOut.a = 0.2f * Clamp01MapToRange(0.3f, particle->colorOut.a, 0.2f);
            }
        }

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

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
            RenderScene(game);

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

            RenderText(&game->fpsCounter);

            for(int i = 0; i < ArrayCount(particles); ++i)
            {
                RenderParticle(&particles[i], &particleTexture, game->particleShader, 0.3f);
            }

            glDisable(GL_BLEND);
        }
        else
        {
            RenderTextDemo(game);
        }

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

        char dynamicBuffer[20];
        sprintf(dynamicBuffer, "%.5f FPS", fps);

        UpdateText(&game->fpsCounter, dynamicBuffer);

        game->lastFrame = thisFrame;
    }

    SDL_Log("Exited the main loop\n");


    return 0;
}

