#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

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

#include "light.cpp"
#include "camera.cpp"
#include "shader.cpp"
#include "texture.cpp"
#include "mesh.cpp"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL.h>

#include <stdio.h>
#include <vector>

int main(int argc, char *argv[])
{
    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    LoadAssets(game);


    //Font font = PrepareFont("../data/fonts/arial.ttf", 36);
    Font font = PrepareFont("../data/fonts/Roboto-Regular.ttf", 36);

    DynamicText dynTextTest = CreateDynamicText(&font, "AAAaqApAPA aaaap BBBb )( (Dynamic)", vec2(0.0f, 200.0f), game->uiDynamicTextShader);
    StaticText staticTextTest = CreateStaticText(game, "AAAaqApAPA aaaap BBBb )( (Static)", vec2(0, 250), game->uiStaticTextShader, 36);

    game->staticTextCounter = CreateStaticText(game, "0 (static)", vec2(20, 36), game->uiStaticTextShader, 36);
    game->dynamicTextCounter = CreateDynamicText(&font, "0 (dynamic)", vec2(250.0f, 36.0f), game->uiDynamicTextShader);

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateGame(game);

        //Rendering
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
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineTexture, 0);
        RenderScene(game);
        game->outlinePass = false;

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture, 0);
        RenderScene(game);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        SetTexture(game->outlineTexture, 0);
        ShaderSetInt(game->postProcessShader, "u_outline", 0);
        SetTexture(game->fullSceneTexture, 1);
        ShaderSetInt(game->postProcessShader, "u_scene", 1);

        glBindVertexArray(game->fullscreenQuad.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        RenderStaticText(&game->staticTextCounter);
        RenderDynamicText(&game->dynamicTextCounter);

        for(int i = 0; i < game->texts.size(); i++)
        {
            RenderStaticText(&game->texts[i]);
        }

        RenderDynamicText(&dynTextTest);
        RenderStaticText(&staticTextTest);

        glDisable(GL_BLEND);

        SDL_GL_SwapWindow(game->window);

        Uint64 thisFrame = SDL_GetPerformanceCounter();
        if(game->lockFPS)
        {
            int targetFrames = 60;
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

        game->lastFrame = thisFrame;
    }

    SDL_Log("Exited the main loop\n");


    return 0;
}

