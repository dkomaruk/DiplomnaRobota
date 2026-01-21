#define GLM_ENABLE_EXPERIMENTAL
#define IMGUI_DEFINE_MATH_OPERATORS

//TODO: Replace with SDL_ShowOpenFileDialog
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

//Has to be included at the start because of compilation errors otherwise
//TODO: Replace with a less heavy library
#include <json.hpp>

//GLM extensions have to be included before any other glm header file for some reason
#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>

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
#include "terrain.cpp"

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

    GLuint smokeFBO;
    Texture smokeTexture = {};
    smokeTexture.x = (int)(WINDOW_WIDTH / 2.0f);
    smokeTexture.y = (int)(WINDOW_HEIGHT / 2.0f);

    Texture smokeDepthTexture = smokeTexture;

    glGenFramebuffers(1, &smokeFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, smokeFBO);

    glGenTextures(1, &smokeTexture.id);
    glBindTexture(GL_TEXTURE_2D, smokeTexture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, smokeTexture.x, smokeTexture.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smokeTexture.id, 0);

    glGenTextures(1, &smokeDepthTexture.id);
    glBindTexture(GL_TEXTURE_2D, smokeDepthTexture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, smokeDepthTexture.x, smokeDepthTexture.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, smokeDepthTexture.id, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //TERRAIN
    std::vector<Vertex> lineVertices = {
        Vertex{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
    };

    Mesh line = CreateMesh(lineVertices);
    MaterialPhong lineMat = {};
    lineMat.shader = game->lightSourceShader;

    game->soldierEntity->position.x = 0.0f;
    game->soldierEntity->position.z = 0.0f;

    glm::vec2 target = {0.0f, 0.0f};
    glm::vec2 targetDirection = {0.0f, 0.0f};

    {
        //Need to do this before the game loop because calling glReadPixels for the first time
        //for some reason causes a huge freeze (174 ms unoptimized). Subsequent calls are 10-12 ms
        //This is not a good solution but it's a workaround
        glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo);
        uint8 pixels[3];
        glReadPixels((int)0, (int)0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateEditorUI(game);
        UpdateGame(game);

        float time = SDL_GetTicks() / 1000.0f;

        float x = game->soldierEntity->position.x + targetDirection.x * 10.0f * game->deltaTime;
        float z = game->soldierEntity->position.z + targetDirection.y * 10.0f * game->deltaTime;
        float y = GetTerrainHeight(&game->terrain, x, z);

        game->soldierEntity->position = glm::vec3(x, y, z);

        if(glm::distance(glm::vec2(x, z), target) < 0.1f)
        {
            targetDirection = glm::vec2(0, 0);
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

                uint64 start = SDL_GetPerformanceCounter();
                uint8 pixels[3];
                glReadPixels((int)x, (int)y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
                uint32 pickedID = pixels[0];
                uint64 end = SDL_GetPerformanceCounter();
                SDL_Log("%f ms", ((end - start) / (float)game->perfFreq) * 1000.0f);

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

                glm::vec3 windowPos = glm::vec3(x, y, 0.0f);
                glm::vec3 rayNear = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                                   glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));
                windowPos.z = 1.0f;
                glm::vec3 rayFar = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                                  glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));

                glm::vec3 rayDirection = glm::normalize(rayFar - rayNear);
                //SDL_Log("%f %f %f", rayDirection.x, rayDirection.y, rayDirection.z);

                glm::vec3 rayOrigin = game->camera.position;
                lineVertices[0].position = glm::vec3(rayOrigin);
                lineVertices[1].position = glm::vec3(rayOrigin + rayDirection * 200.0f);
                UpdateMesh(&line, lineVertices);

                glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, rayOrigin, rayDirection, 200.0f);
                //SDL_Log("Intersection: %f %f %f", intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);

                target = glm::vec2(intersectionPoint.x, intersectionPoint.z);
                targetDirection = glm::normalize(target - glm::vec2(game->soldierEntity->position.x, game->soldierEntity->position.z));
            }

            game->outlinePass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineTexture.id, 0);
            RenderScene(game);
            game->outlinePass = false;

            glDepthMask(GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
            RenderScene(game);

            static GLenum terrainDisplayMode = GL_FILL;
            if(IsFirstPress(game, SDL_SCANCODE_SPACE))
            {
                terrainDisplayMode = (terrainDisplayMode == GL_LINE) ? GL_FILL : GL_LINE;
            }

            glPolygonMode(GL_FRONT_AND_BACK, terrainDisplayMode);
            RenderTerrain(game);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            ShaderSetVec3(game->lightSourceShader, "u_lightColor", 1.0f, 0.0f, 0.0f);
            RenderMesh(game, &line, &lineMat, glm::mat4(1.0f), GL_LINES);

            if(game->renderParticles)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, smokeFBO);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glViewport(0, 0, smokeTexture.x, smokeTexture.y);
                SetTexture(&game->fullSceneDepthTexture, 2);
                ShaderSetInt(game->particleShader, "u_sceneDepth", 2);
                ShaderSetVec2(game->particleShader, "u_screenSize", (float)smokeTexture.x, (float)smokeTexture.y);
                RenderParticles(game);
                glViewport(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            SetTexture(&game->outlineTexture, 0);
            ShaderSetInt(game->postProcessShader, "u_outline", 0);
            SetTexture(&game->fullSceneTexture, 1);
            ShaderSetInt(game->postProcessShader, "u_scene", 1);
            SetTexture(&smokeTexture, 2);
            ShaderSetInt(game->postProcessShader, "u_smoke", 2);
            SetTexture(&game->fullSceneDepthTexture, 3);
            ShaderSetInt(game->postProcessShader, "u_sceneDepth", 3);
            SetTexture(&smokeDepthTexture, 4);
            ShaderSetInt(game->postProcessShader, "u_smokeDepth", 4);

            ShaderSetVec2(game->postProcessShader, "u_lowResInvSize", 1.0f / smokeTexture.x, 1.0f / smokeTexture.y);

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

