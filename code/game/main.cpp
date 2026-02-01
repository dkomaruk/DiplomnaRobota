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
#include "model.cpp"
#include "animation.cpp"
#include "particle_system.cpp"
#include "particle_editor_ui.cpp"
#include "terrain.cpp"
#include "framebuffer.cpp"
#include "primitives/line.cpp"

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

    //Model *model = ImportModel("../data/models/soldier/vampire/vampire.fbx", game->animationShader, aiProcess_Triangulate | aiProcess_GlobalScale, ModelType_Animated, 0.01f);
    Model *model = ImportModel("../data/models/soldier/Rifle Run.fbx", game->animationShader, aiProcess_Triangulate | aiProcess_GlobalScale, ModelType_Animated, 0.01f);

    Line line = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), game->lineShader, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec2 target = {0.0f, 0.0f};
    glm::vec2 targetDirection = {0.0f, 0.0f};

    {
        //Need to do this before the game loop because calling glReadPixels for the first time
        //for some reason causes a huge freeze (174 ms unoptimized compilation). Subsequent calls are 10-12 ms
        //Maybe I should just do ray picking instead of using a framebuffer with IDs
        glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo.id);
        uint8 pixels[3];
        glReadPixels((int)0, (int)0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //Model *abramsTurret = ImportModel2("../data/models/abrams/abrams.fbx", game->mainShader);
    //Model *abrams = ImportModel("../data/models/abrams/abrams.fbx", game->mainShader, aiProcess_PreTransformVertices);
    //if(!abramsTurret)
    //{
    //    SDL_Log("Failed to load abrams.fbx");
    //}

    //Entity tankTurret = CreateEntity(abramsTurret);
    //tankTurret.position.y += 5.0f;
    //tankTurret.id = game->sceneEntities.back()->id + 1;
    //game->sceneEntities.push_back(&tankTurret);

    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 angularVelocity = glm::vec3(0.0f);

    Entity vampire = CreateEntity(model);
    game->soldierEntity = &vampire;
    game->soldierEntity->position.x = 0.0f;
    game->soldierEntity->position.z = 0.0f;

    game->lastFrame = SDL_GetPerformanceCounter();
    while(game->isRunning)
    {
        //Input
        ProcessInput(game);

        //Update
        UpdateParticleEditorUI(game);
        UpdateGame(game);

        UpdateAnimation(&model->animData, game->deltaTime);

        //UpdateAnimation(&animation, animTime, skinningMatrices);

        //tankTurret.position += velocity * game->deltaTime + 0.5f * acceleration * Square(game->deltaTime);
        //tankTurret.rotation += angularVelocity * game->deltaTime;

        if(IsFirstPress(game, SDL_SCANCODE_R))
        {
            velocity = glm::vec3(RandomBetween(-2.0f, 2.0f), 25.0f, RandomBetween(-2.0f, 2.0f));
            acceleration.y = -9.8f;
            angularVelocity = glm::vec3(RandomBetween(-180.0f, 180.0f),
                                        RandomBetween(-180.0f, 180.0f),
                                        RandomBetween(-180.0f, 180.0f));
        }

        angularVelocity -= angularVelocity * 0.3f * game->deltaTime;
        velocity += acceleration * game->deltaTime;

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
            glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo.id);
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
                //This is very expensive, I think AABB ray intersection will be much more cheap
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
                glm::vec3 rayOrigin = game->camera.position;

                float visibleRayLength = 200.0f;

                glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, rayOrigin, rayDirection, visibleRayLength);

                UpdateLine(&line, rayOrigin, rayOrigin + rayDirection * visibleRayLength);

                target = glm::vec2(intersectionPoint.x, intersectionPoint.z);
                targetDirection = target - glm::vec2(game->soldierEntity->position.x, game->soldierEntity->position.z);

                //Prevents silent division by zero in the glm::normalize and NaN in the targetDirection as a result
                if(glm::length2(targetDirection) > 0.00001f)
                {
                    targetDirection = glm::normalize(targetDirection);
                }
            }

            game->outlinePass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineFbo.color.id, 0);
            RenderScene(game);
            game->outlinePass = false;

            glDepthMask(GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
            ShaderSetVec3(game->lightSourceShader, "u_color", glm::vec3(1.0f));
            RenderScene(game);

            UseShader(game->animationShader);
            ShaderSetMatrix4Array(game->animationShader, "u_skinning", glm::value_ptr(model->animData.skinningMatrices[0]), 100);
            RenderEntity(game->soldierEntity, game);
            //RenderModel(game, model, glm::mat4(1.0f));

            //RenderModel(game, abramsTurret, PrepareModelMatrix(tankTurret.position, tankTurret.rotation, tankTurret.scale));

            static GLenum terrainDisplayMode = GL_FILL;
            if(IsFirstPress(game, SDL_SCANCODE_SPACE))
            {
                terrainDisplayMode = (terrainDisplayMode == GL_LINE) ? GL_FILL : GL_LINE;
            }


            RenderLine(&line);
            glPolygonMode(GL_FRONT_AND_BACK, terrainDisplayMode);
            RenderTerrain(game);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if(game->renderParticles)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, game->smokeFbo.id);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glViewport(0, 0, game->smokeFbo.color.x, game->smokeFbo.color.y);
                SetTexture(&game->fullSceneDepthTexture, 2);
                ShaderSetInt(game->particleShader, "u_sceneDepth", 2);
                ShaderSetVec2(game->particleShader, "u_screenSize", game->smokeFbo.color.size);
                RenderParticles(game);
                glViewport(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            SetTexture(&game->outlineFbo.color, 0);
            ShaderSetInt(game->postProcessShader, "u_outline", 0);
            SetTexture(&game->fullSceneTexture, 1);
            ShaderSetInt(game->postProcessShader, "u_scene", 1);
            SetTexture(&game->smokeFbo.color, 2);
            ShaderSetInt(game->postProcessShader, "u_smoke", 2);
            SetTexture(&game->fullSceneDepthTexture, 3);
            ShaderSetInt(game->postProcessShader, "u_sceneDepth", 3);
            SetTexture(&game->smokeFbo.depth, 4);
            ShaderSetInt(game->postProcessShader, "u_smokeDepth", 4);

            ShaderSetVec2(game->postProcessShader, "u_lowResInvSize", 1.0f / (glm::vec2)game->smokeFbo.color.size);

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
            int targetFrames = 30;
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

        char buffer[20];
        sprintf(buffer, "%.5f FPS", fps);
        UpdateText(&game->fpsCounter, buffer);

        sprintf(buffer, "%.5f ms/f", ms);
        UpdateText(&game->msPerFrame, buffer);

        game->lastFrame = thisFrame;
    }

    return 0;
}

