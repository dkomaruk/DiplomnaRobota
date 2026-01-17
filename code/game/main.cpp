#define GLM_ENABLE_EXPERIMENTAL
#define IMGUI_DEFINE_MATH_OPERATORS

//Has to be included at the start because of compilation errors otherwise
//TODO: Replace with a less heavy library
#include <json.hpp>

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

#include "string_utils.h"

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

#include <time.h>

#include <imgui.h>

#include <expat.h>

#define TEXTURE_ATLAS_ELEMENT 0
#define SPRITE_ELEMENT 1

void XMLCALL StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
    Atlas *atlas = (Atlas *)userData;

    int element = -1;
    if(strcmp(name, "TextureAtlas") == 0)
    {
        element = TEXTURE_ATLAS_ELEMENT;
    }
    else if(strcmp(name, "sprite") == 0)
    {
        element = SPRITE_ELEMENT;
    }

    switch(element)
    {
        case TEXTURE_ATLAS_ELEMENT:
        {
            atlas->size.x = StrToFloat(atts[3]);
            atlas->size.y = StrToFloat(atts[5]);
            SDL_Log("x: %f; y:%f", atlas->size.x, atlas->size.y);
        } break;

        case SPRITE_ELEMENT:
        {
            Sprite sprite = {};
            float pixelX = StrToFloat(atts[3]);
            float pixelY = StrToFloat(atts[5]);
            float pixelW = StrToFloat(atts[7]);
            float pixelH = StrToFloat(atts[9]);

            sprite.pos.x = pixelX / atlas->size.x;
            sprite.pos.y = (atlas->size.y - pixelY - pixelH) / atlas->size.y;

            sprite.size.x = pixelW / atlas->size.x;
            sprite.size.y = pixelH / atlas->size.y;

            atlas->sprites.push_back(sprite);

            SDL_Log("%d. (%f, %f), (%f, %f)", (int)atlas->sprites.size(), sprite.pos.x, sprite.pos.y, sprite.size.x, sprite.size.y);
        } break;

        default:
        {
            //Undefined case
            //Assert(0);
        } break;
    }
}

void XMLCALL EndElement(void *userData, const XML_Char *name) { }
void XMLCALL CharacterData(void *userData, const XML_Char *s, int len) { }

int main(int argc, char *argv[])
{
    srand((uint32)time(0));

    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    size_t fileSize;
    void *fileMemory = SDL_LoadFile("../data/imgs/animated_smoke/1.xml", &fileSize);
    if(!fileMemory)
    {
        SDL_Log("Failed to load 1.xml. Error: %s", SDL_GetError());
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    if(!parser)
    {
        SDL_Log("Failed to create an XML parser");
    }

    Atlas atlas = {};
    XML_SetUserData(parser, (void *)&atlas);

    XML_SetElementHandler(parser, StartElement, EndElement);
    //XML_SetCharacterDataHandler(parser, CharacterData);

    XML_Status parsingResult = XML_Parse(parser, (char *)fileMemory, (int)fileSize, XML_TRUE);
    if(!parsingResult)
    {
        SDL_Log("Failed to parse 1.xml");
    }
    SDL_free(fileMemory);

    LoadAssets(game);

    game->lastFrame = SDL_GetPerformanceCounter();
    //game->lockFPS = true;

    game->particleTextures[0] = CreateTexture("../data/imgs/smoke.png");
    game->particleTextures[1] = CreateTexture("../data/imgs/smoke2.png");
    game->particleTextures[2] = CreateTexture("../data/imgs/smoke3.png");
    game->particleTextures[3] = CreateTexture("../data/imgs/smoke4.png");
    game->particleTextures[4] = CreateTexture("../data/imgs/smoke5.png");
    game->particleTextures[5] = CreateTexture("../data/imgs/animated_smoke/1.png");

    for(int i = 0; i < ArrayCount(game->particleSystems); i++)
    {
        game->particleSystems[i] = InitParticleSystem(game, &game->smokeSettings);
    }

    game->particleSystems[0].pos = glm::vec3(0.0f);

    int maxNumOfParticles = game->smokeSettings.maxNumOfParticles;
    game->particleData = (ParticleData *)calloc(maxNumOfParticles * ArrayCount(game->particleSystems), sizeof(ParticleData));
    game->textureID = game->particleTextures[game->currentTexture].id;

    atlas.path = "../data/imgs/animated_smoke/1.png";
    game->smokeSettings.atlas = &atlas;


    game->particlesQuad = CreateUnitQuadStripes();
    glBindVertexArray(game->particlesQuad.vao);

    glGenBuffers(1, &game->vboInstances);
    glBindBuffer(GL_ARRAY_BUFFER, game->vboInstances);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * maxNumOfParticles * ArrayCount(game->particleSystems),
                 game->particleData, GL_STREAM_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, scale));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, angle));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, uvOffset));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, uvScale));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, offset));
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, color));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
    {
        if(game->smokeSettings.prewarm)
        {
            game->particleSystems[i].prewarmTimer = StartTimer(game->smokeSettings.prewarmSeconds);
        }
    }

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateEditorUI(game);
        UpdateGame(game);

        if(IsFirstPress(game, SDL_SCANCODE_Y))
        {
            game->renderParticles = !game->renderParticles;
        }

        if(game->renderParticles)
        {
            for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
            {
                SpawnParticles(game, &game->particleSystems[i]);
                UpdateParticles(game, &game->particleSystems[i]);
            }

            SortAllParticles(game);
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

            glDepthMask(GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
            RenderScene(game);

            if(game->renderParticles)
            {
                RenderParticles(game);
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

            RenderText(&game->aliveParticlesText);
            RenderText(&game->deadParticlesText);
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

        char buffer[20];
        sprintf(buffer, "%.5f FPS", fps);

        UpdateText(&game->fpsCounter, buffer);

        sprintf(buffer, "%.5f ms/f", ms);
        UpdateText(&game->msPerFrame, buffer);

        game->lastFrame = thisFrame;
    }

    SDL_Log("Exited the main loop\n");


    return 0;
}

