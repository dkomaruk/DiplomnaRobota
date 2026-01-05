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
#include "image.cpp"
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

    Font font = PrepareFont("../data/fonts/Roboto-Regular.ttf", 36);
    Font font2 = PrepareFont("../data/fonts/Roboto-Regular.ttf", 18);

    char *text1 = "AAAaqApAPA aaapaIp I BBBb )(";
    char *text3 = "kerning This is just a bunch of text here";

    Text textTest = CreateText(&font, text1, vec2(0.0f, 200.0f), game->uiTextShader, vec3(1.0f, 0.0f, 0.0f));
    Text textTest2 = CreateText(&font, text3, vec2(0.0f, 280.0f), game->uiTextShader, vec3(1.0f));

    game->fpsCounter = CreateText(&font2, "0 FPS", vec2(20.0f, 36.0f), game->uiTextShader);
    game->helloWorldsCounterDisplay = CreateText(&font2, "0 (hello worlds)", vec2(300.0f, 36.0f), game->uiTextShader);

    game->textInput = CreateText(&font, (char *)game->textInputBuffer.c_str(), vec2(10.0f,  400.0f),
                                        game->uiTextShader, vec3(1.0f));

    game->textStatus = CreateText(&font, "Text input: disabled", vec2(WINDOW_WIDTH - 500.0f,  36.0f),
                                  game->uiTextShader, vec3(1.0f));

    char text[] = "again place well so she change what out tell against know line stand it end like home hold develop while under tell such large move some it those mean many even school by can give keep seem out large such system have feel use keep here this know like";

    Timer inputTimer = {};
    Timer pauseTimer = {};
    inputTimer = StartTimer(15);

    game->lastFrame = SDL_GetPerformanceCounter();

    //game->lockFPS = true;

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateGame(game);

        if(!game->typingText)
        {
            UpdateTimer(&inputTimer, game->deltaTime);
            game->textChanged = true;
        }


        if(game->textChanged)
        {
            game->textChanged = false;
            if(game->typingText)
            {
                UpdateText(&game->textInput, (char *)game->textInputBuffer.c_str());
            }
            else
            {
                int lettersOut = (int)((sizeof(text) - 1) * (inputTimer.elapsed / inputTimer.duration));
                UpdateText(&game->textInput, text, lettersOut);
            }
        }

        if(inputTimer.isFinished)
        {
            if(pauseTimer.isFinished)
                pauseTimer = StartTimer(3);
            else
                UpdateTimer(&pauseTimer, game->deltaTime);


            if(pauseTimer.isFinished)
                inputTimer = StartTimer(15);
        }

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

        RenderText(&textTest);
        RenderText(&textTest2);

        RenderText(&game->fpsCounter);

        RenderText(&game->helloWorldsCounterDisplay);

        if(game->helloWorldsCounter)
        {
            RenderText(&game->helloWorlds);
        }

        RenderText(&game->textStatus);
        RenderText(&game->textInput);

        glDisable(GL_BLEND);

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

