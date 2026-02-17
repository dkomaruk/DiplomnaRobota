#define GLM_ENABLE_EXPERIMENTAL
#define IMGUI_DEFINE_MATH_OPERATORS

//TODO: Replace with SDL_ShowOpenFileDialog
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#undef near
#undef far

//Has to be included at the start because of compilation errors otherwise
//TODO: Replace with a less heavy library
#include <json.hpp>

//GLM extensions have to be included before any other glm header file for some reason, otherwise it doesn't compile
#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/vector_angle.hpp>

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
#include "file.cpp"
#include "shader.cpp"
#include "aabb.cpp"
#include "texture.cpp"
#include "image.cpp"
#include "mesh.cpp"
#include "model.cpp"
#include "animation.cpp"
#include "particle_system.cpp"
#include "particle_editor_ui.cpp"
#include "editor_ui.cpp"
#include "terrain.cpp"
#include "framebuffer.cpp"
#include "ui.cpp"
#include "line.cpp"
#include "frustum.cpp"
#include "plane.cpp"
#include "ray.cpp"
#include "selection.cpp"

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

    Model *model = ImportModel("../data/models/soldier/Rifle Walk.fbx", game->animationShader, aiProcess_Triangulate | aiProcess_GlobalScale, ModelType_Animated, 0.01f);

    game->pickingRay = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), game->lineShader, glm::vec3(1.0f, 0.0f, 0.0f));
    CreateFrustumLines(game->frustumLines, game->frustumNormals, game->lineShader);

    Model *abrams = ImportModel("../data/models/abrams/abrams.fbx", game->mainShader, 0);
    if(!abrams)
    {
        SDL_Log("Failed to load abrams.fbx");
    }

    Entity tank = CreateEntity(abrams);
    for(int i = 0; i < tank.model->numOfNodes; i++)
    {
        char *nodeName = tank.model->nodes[i].name;
        if(nodeName && strcmp(nodeName, "Tourelle_01") == 0)
        {
            tank.turret.nodeId = i;
            tank.turret.transform = tank.model->nodes[i].localTransform;
        }
        else if(nodeName && strcmp(nodeName, "Axe_Canon_01") == 0)
        {
            tank.gun.nodeId = i;
            tank.gun.transform = tank.model->nodes[i].localTransform;
        }
    }

    tank.id = game->sceneEntities.back()->id + 1;
    strcpy(tank.textId, "tank");
    game->sceneEntities.push_back(&tank);

    Entity character = CreateEntity(model);
    game->soldierEntity = &character;
    game->soldierEntity->position.x = 0.0f;
    game->soldierEntity->position.z = 0.0f;
    game->soldierEntity->id = game->sceneEntities.back()->id + 1;
    game->sceneEntities.push_back(game->soldierEntity);

    //SKY
    int flags = TexturePreset_Common;
    flags = FLAG_TOGGLE(flags, TextureFlag_Filter_Min_LinLin | TextureFlag_Filter_Min_Nearest | TextureFlag_FlipY);
    Texture skyTexture = CreateTexture("../data/imgs/extra/sky.png", flags);
    Mesh quad = CreateQuadNDC(glm::vec2(0.0f), glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
    GLuint shader = CreateShaderProgram(LoadShader("../data/shaders/environment.vert"),
                                        LoadShader("../data/shaders/environment.frag"));

    //MAIN GAME LOOP START
    game->lastFrame = SDL_GetPerformanceCounter();
    while(game->isRunning)
    {
        //Input
        ProcessInput(&game->input);

        //Update
        UpdateEditorUI(game);
        UpdateGame(game);

        if(IsFirstPress(game, SDL_SCANCODE_DELETE))
        {
            game->sceneEntities.erase(
                std::remove_if(game->sceneEntities.begin(), game->sceneEntities.end(), [&](Entity *entity) {
                    if(game->selectedIDs.count(entity->id))
                    {
                        DeleteEntity(entity);
                        return true;
                    }
                    return false;
                }),
                game->sceneEntities.end()
            );

            game->selectedIDs.clear();
            game->lastSelectedId = -1;
        }

        glm::mat4 turretTransform = glm::mat4(1.0f);
        turretTransform = glm::translate(turretTransform, glm::vec3(0.0f, 0.0f, 0.25f + (sinf((float)SDL_GetTicks() / 1000.0f) + 1.0f) / 2.0f));
        turretTransform = glm::rotate(turretTransform, glm::radians(45.0f * (SDL_GetTicks() / 1000.0f)), glm::vec3(0.0f, 0.0f, 1.0f));
        tank.turret.transform = tank.model->nodes[tank.turret.nodeId].localTransform * turretTransform;

        glm::mat4 gunTransform = glm::mat4(1.0f);
        gunTransform = glm::rotate(gunTransform, glm::radians(sinf(SDL_GetTicks() / 500.0f) * 20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        tank.gun.transform = tank.model->nodes[tank.gun.nodeId].localTransform * gunTransform;

        UpdateTransforms(&tank);

        //glm::mat4 tankWorldMatrix = PrepareModelMatrix(tank.position, tank.rotation, tank.scale);
        //glm::mat4 tipWorldMat = tankWorldMatrix * tank.nodeTransforms[tank.gunTipId];
        //game->particleSystems[0].pos = tipWorldMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        //glm::mat4 tipRotation = glm::mat3(tipWorldMat);
        //game->particleSystems[0].rotation = tipRotation;

        float speed = 1.0f;
        float x = game->soldierEntity->position.x + game->targetDirection.x * speed * game->deltaTime;
        float z = game->soldierEntity->position.z + game->targetDirection.y * speed * game->deltaTime;
        float y = GetTerrainHeight(&game->terrain, x, z);

        game->soldierEntity->position = glm::vec3(x, y, z);

        float angleDiff = game->targetAngle - game->soldierEntity->rotation.y;

        if(angleDiff < -180.0f)
            angleDiff += 360.0f;
        if(angleDiff > 180.0f)
            angleDiff -= 360.0f;

        float rotationStep = 200.0f * game->deltaTime;

        if(glm::abs(angleDiff) <= rotationStep)
            game->soldierEntity->rotation.y = game->targetAngle;
        else
            game->soldierEntity->rotation.y += glm::sign(angleDiff) * rotationStep;

        if(glm::distance(glm::vec2(x, z), game->target) < 0.1f)
        {
            game->targetDirection = glm::vec2(0, 0);
        }

        //RENDERING
        if(!game->textDemoEnabled)
        {
            glEnable(GL_DEPTH_TEST);

            game->outlinePass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineFbo.color.id, 0);
            RenderScene(game);
            game->outlinePass = false;

            static GLenum terrainDisplayMode = GL_FILL;
            if(IsFirstPress(game, SDL_SCANCODE_SPACE))
            {
                terrainDisplayMode = (terrainDisplayMode == GL_LINE) ? GL_FILL : GL_LINE;
            }
            glPolygonMode(GL_FRONT_AND_BACK, terrainDisplayMode);

            glDepthMask(GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
            ShaderSetVec3(game->lightSourceShader, "u_color", glm::vec3(1.0f));
            RenderScene(game);

#ifdef DEBUG
            if(game->renderPickingRay)
            {
                RenderLine(&game->pickingRay);
            }
            if(game->renderSelectionFrustum)
            {
                for(int i = 0; i < ArrayCount(game->frustumLines); i++)
                {
                    RenderLine(&game->frustumLines[i]);
                }
                for(int i = 0; i < ArrayCount(game->frustumNormals); i++)
                {
                    RenderLine(&game->frustumNormals[i]);
                }
            }
#endif

            if(game->renderTerrain)
            {
                RenderTerrain(game);
            }

            UseShader(shader);
            glDepthFunc(GL_LEQUAL);

            ShaderSetMatrix4(shader, "u_viewProjInverse", game->projViewInverse);

            SetTexture(skyTexture.id, 0);
            ShaderSetInt(shader, "u_skyMap", 0);
            glBindVertexArray(quad.vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glDepthFunc(GL_LESS);

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

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

            if(game->input.mouseButtons[MOUSE_LEFT] && RECT_HAS_SIZE(game->selectionBox.size))
            {
                RenderSelectionBox(game, &game->selectionBox);
            }

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

