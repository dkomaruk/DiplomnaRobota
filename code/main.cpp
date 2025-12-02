#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

#include "external/stb_image.cpp"
#include "external/stb_image_write.cpp"

#include "infantry.cpp"
#include "entity.cpp"
#include "input.cpp"
#include "game.cpp"
#include "audio.cpp"
#include "asset_loader.cpp"

#include "util/timer.cpp"

#include "graphics/light.cpp"
#include "graphics/camera.cpp"
#include "graphics/shader.cpp"
#include "graphics/texture.cpp"
#include "graphics/mesh.cpp"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL.h>

#include <SDL3_ttf/SDL_ttf.h>

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

    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f,     0.0f, 0.0f, 0.0f,     0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f, 0.0f,     0.0f, 0.0f,
        1.0f, -1.0f,  0.0f,     0.0f, 0.0f, 0.0f,    1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,     0.0f, 0.0f, 0.0f,     0.0f, 1.0f,
        1.0f, -1.0f,  0.0f,     0.0f, 0.0f, 0.0f,    1.0f, 0.0f,
        1.0f,  1.0f,  0.0f,     0.0f, 0.0f, 0.0f,    1.0f, 1.0f
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(0 * sizeof(float)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    //Mesh quad = CreateQuad(vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f), vec2(100.0f, 50.0f), game->uiShader);

    float lastQuadX = 150.0f;
    float lastQuadY = 0.0f;

    //GLuint faceTexture = CreateTexture("../data/imgs/face.png");
    SDL_Surface *textSurface = TTF_RenderText_Blended(game->font, "Hello, world!", 0, SDL_Color{255, 255, 255, 255});
    SDL_FlipSurface(textSurface, SDL_FLIP_VERTICAL);

    vec2 textSize = vec2(textSurface->w, textSurface->h) * 2.0f;

    GLuint faceTexture = CreateGLTexture((uint8 *)textSurface->pixels, textSurface->pitch / 4, textSurface->h, true, true);
    std::vector<Mesh> quads = {
        //CreateQuad(vec2(0.0f, 0.0f), vec2(50.0f, 50.0f), game->uiShader),
        //CreateQuad(vec2(50.0f, 0.0f), vec2(50.0f, 50.0f), game->uiShader),
        //CreateQuad(vec2(100.0f, 0.0f), vec2(50.0f, 50.0f), game->uiShader),
        //CreateQuad(vec2(150.0f, 0.0f), vec2(50.0f, 50.0f), game->uiShader)
        CreateQuad(vec2(100.0f, 100.0f), textSize, game->uiShader)
    };

    for(int i = 0; i < quads.size(); i++)
    {
        quads[i].material.diffuseTexture = faceTexture;
    }
    ShaderSetInt(game->uiShader, "u_texture", 0);

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateGame(game);

        Camera *camera = &game->camera;
        vec3 forward = normalize(camera->direction);

        vec3 worldUp = vec3(0.0f, 1.0f, 0.0f);
        vec3 right = normalize(cross(forward, worldUp));
        vec3 up = normalize(cross(right, forward));

        ALfloat listenerOri[6] = {
            camera->direction.x, camera->direction.y, camera->direction.z,
            up.x, up.y, up.z
        };
        alListener3f(AL_POSITION, camera->position.x, camera->position.y, camera->position.z);
        alListenerfv(AL_ORIENTATION, listenerOri);

        if(game->keys[SDL_SCANCODE_DOWN])
        {
            game->outlineThickness -= 5.0f * game->deltaTime;
            ShaderSetInt(game->postProcessShader, "u_outlineThickness", (int)game->outlineThickness);
        }
        if(game->keys[SDL_SCANCODE_UP])
        {
            game->outlineThickness += 5.0f * game->deltaTime;
            ShaderSetInt(game->postProcessShader, "u_outlineThickness", (int)game->outlineThickness);
        }

        if(IsFirstPress(game, SDL_SCANCODE_SPACE))
        {
            int sourceState;
            alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
            if(sourceState == AL_PLAYING)
            {
                alSourcePause(source);
            }
            else
            {
                alSourcePlay(source);
            }
        }
        if(IsFirstPress(game, SDL_SCANCODE_U))
        {
            int w = (int)WINDOW_WIDTH;
            int h = (int)WINDOW_HEIGHT;
            int bytesPerPixel = 3;

            uint8 *pixels = (uint8 *)malloc(w * h * bytesPerPixel);
            glBindFramebuffer(GL_FRAMEBUFFER, pickingFbo);
            glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
            stbi_write_png("test2.png", w, h, bytesPerPixel, pixels, w * bytesPerPixel);
            free(pixels);
        }
        if(IsFirstPress(game, SDL_SCANCODE_P))
        {
            if(game->isCursorHidden)
            {
                SDL_ShowCursor();
                SDL_SetWindowRelativeMouseMode(game->window, false);
            }
            else
            {
                SDL_HideCursor();
                SDL_SetWindowRelativeMouseMode(game->window, true);
            }

            game->isCursorHidden = !game->isCursorHidden;
        }

        //if(IsFirstPress(game, SDL_SCANCODE_UP))
        if(game->keys[SDL_SCANCODE_UP])
        {
            float newQuadX = lastQuadX + textSize.x;
            float newQuadY = lastQuadY;
            if(newQuadX >= WINDOW_WIDTH)
            {
                newQuadX = 0.0f;
                newQuadY += textSize.y + 10.0f;
            }

            Mesh newQuad = CreateQuad(vec2(newQuadX, newQuadY), textSize, game->uiShader);
            newQuad.material.diffuseTexture = faceTexture;
            quads.push_back(newQuad);

            lastQuadX = newQuadX;
            lastQuadY = newQuadY;
        }

        for(int i = 0; i < MOUSE_BUTTONS_COUNT; i++)
        {
            if(IsFirstClick(game, i))
            {
                SDL_Log("%s", GetMouseButtonName(i));
            }
        }

        game->testEntity->rotation.y = (float)SDL_GetTicks() / 25.0f;

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

        glUseProgram(game->postProcessShader);
        ShaderSetFloat(game->postProcessShader, "u_time", (float)SDL_GetTicks() / 1000.0f);

        SetTexture(game->outlineTexture, 0);
        ShaderSetInt(game->postProcessShader, "u_outline", 0);
        SetTexture(game->fullSceneTexture, 1);
        ShaderSetInt(game->postProcessShader, "u_scene", 1);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for(int i = 0; i < game->quads.size(); i++)
        {
            RenderMesh(game, &game->quads[i], mat4(1.0f));
        }
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
        SDL_Log("%f", fps);

        game->lastFrame = thisFrame;
    }

    return 0;
}

