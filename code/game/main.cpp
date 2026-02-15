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

void RenderRectUI(Game *game, glm::vec2 pos, glm::vec2 size, GLuint shader)
{
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(pos.x + (size.x / 2.0f), pos.y + (size.y / 2.0f), 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(size.x, size.y, 1.0f));
    RenderMesh(game, GetUnitQuad(), modelMat, shader);
}

enum FrustumPoint
{
    FrustumPoint_BottomLeft,
    FrustumPoint_BottomRight,
    FrustumPoint_UpperLeft,
    FrustumPoint_UpperRight,
};

enum FrustumPlane
{
    FrustumPlane_Left,
    FrustumPlane_Right,
    FrustumPlane_Up,
    FrustumPlane_Bottom,
    FrustumPlane_Near,
    FrustumPlane_Far
};

struct Plane
{
    float d;
    glm::vec3 normal;
};

Plane CreatePlane(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    Plane result = {};

    result.normal = glm::normalize(glm::cross(b - a, c - a));
    result.d = -glm::dot(result.normal, a);

    return result;
}

float PointPlaneDistance(Plane *plane, glm::vec3 point)
{
    return glm::dot(plane->normal, point) + plane->d;
}

bool FrustumAABBIntersectionTest(Plane* planes, AABB *aabb)
{
    for(int i = 0; i < 6; i++)
    {
        glm::vec3 p = aabb->min;
        if(planes[i].normal.x >= 0) p.x = aabb->max.x;
        if(planes[i].normal.y >= 0) p.y = aabb->max.y;
        if(planes[i].normal.z >= 0) p.z = aabb->max.z;

        if(PointPlaneDistance(&planes[i], p) < 0)
            return false;
    }

    return true;
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

    //Model *model = ImportModel("../data/models/soldier/vampire/vampire.fbx", game->animationShader, aiProcess_Triangulate | aiProcess_GlobalScale, ModelType_Animated, 0.01f);
    Model *model = ImportModel("../data/models/soldier/Rifle Walk.fbx", game->animationShader, aiProcess_Triangulate | aiProcess_GlobalScale, ModelType_Animated, 0.01f);

    Line line = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), game->lineShader, glm::vec3(1.0f, 0.0f, 0.0f));

    Line frustumLines[4];
    for(int i = 0; i < ArrayCount(frustumLines); i++)
    {
        frustumLines[i] = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), game->lineShader, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    Line frustumNormals[6];
    for(int i = 0; i < ArrayCount(frustumNormals); i++)
    {
        frustumNormals[i] = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), game->lineShader, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec2 target = {0.0f, 0.0f};
    glm::vec2 targetDirection = {0.0f, 0.0f};
    float targetAngle = 0.0f;

#if 0
    {
        //Need to do this before the game loop because calling glReadPixels for the first time
        //for some reason causes a huge freeze (174 ms unoptimized compilation). Subsequent calls are 10-12 ms
        //Maybe I should just do ray picking instead of using a framebuffer with IDs
        glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo.id);
        uint8 pixels[3];
        glReadPixels((int)0, (int)0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
#endif

    //Model *abramsTurret = ImportModel2("../data/models/abrams/abrams.fbx", game->mainShader);
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
        //else if(nodeName && strcmp(nodeName, "Fx_Tourelle1_Tir_01") == 0)
        //{
        //    tank.gunTipId = i;
        //}
    }

    tank.id = game->sceneEntities.back()->id + 1;
    strcpy(tank.textId, "tank");
    game->sceneEntities.push_back(&tank);

    //Entity tankTurret = CreateEntity(abramsTurret);
    //tankTurret.position.y += 5.0f;
    //tankTurret.id = game->sceneEntities.back()->id + 1;
    //game->sceneEntities.push_back(&tankTurret);

    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 angularVelocity = glm::vec3(0.0f);

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

    Mesh *selectionQuad = GetUnitQuad();

    glm::vec2 selectionBoxStart = glm::vec2(0.0f);
    glm::vec2 selectionBoxSize = glm::vec2(0.0f);

    //MAIN GAME LOOP START
    game->lastFrame = SDL_GetPerformanceCounter();
    while(game->isRunning)
    {
        //Input
        ProcessInput(&game->input);

        Input tempInputCopy = game->input;

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

        float speed = 1.0f;
        float x = game->soldierEntity->position.x + targetDirection.x * speed * game->deltaTime;
        float z = game->soldierEntity->position.z + targetDirection.y * speed * game->deltaTime;
        float y = GetTerrainHeight(&game->terrain, x, z);

        game->soldierEntity->position = glm::vec3(x, y, z);

        float angleDiff = targetAngle - game->soldierEntity->rotation.y;

        if(angleDiff < -180.0f)
            angleDiff += 360.0f;
        if(angleDiff > 180.0f)
            angleDiff -= 360.0f;

        float rotationStep = 200.0f * game->deltaTime;

        if(glm::abs(angleDiff) <= rotationStep)
            game->soldierEntity->rotation.y = targetAngle;
        else
            game->soldierEntity->rotation.y += glm::sign(angleDiff) * rotationStep;

        if(glm::distance(glm::vec2(x, z), target) < 0.1f)
        {
            targetDirection = glm::vec2(0, 0);
        }

        //RENDERING
        if(!game->textDemoEnabled)
        {
            glEnable(GL_DEPTH_TEST);

#if 0
            game->pickingPass = true;
            glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo.id);
            RenderScene(game);
            game->pickingPass = false;
#endif

            //OBJECT PICKING
            if(IsFirstClick(game, MOUSE_LEFT) && !ImGui::GetIO().WantCaptureMouse)
            {
                uint64 start = SDL_GetPerformanceCounter();
                glm::vec2 mousePos;
                if(game->input.isCursorHidden)
                {
                    mousePos /= glm::vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
                    selectionBoxStart = mousePos;
                }
                else
                {
                    mousePos = tempInputCopy.mousePos;
                    selectionBoxStart = mousePos;

                    mousePos.y = (int)WINDOW_HEIGHT - mousePos.y;
                }

#if 0
                uint64 start = SDL_GetPerformanceCounter();
                uint8 pixels[3];
                //This is very expensive (because we have to wait for the pixel data request to reach the GPU and then for the data to get back), I think AABB ray intersection with spatial partitioning will be much more cheap
                glReadPixels((int)x, (int)y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixels);
                uint32 pickedID = pixels[0];
                uint64 end = SDL_GetPerformanceCounter();
                SDL_Log("%f ms", ((end - start) / (float)game->perfFreq) * 1000.0f);
#endif

                //uint64 start = SDL_GetPerformanceCounter();
                glm::vec3 windowPos = glm::vec3(mousePos, 0.0f);

                glm::vec3 rayNear = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                                   glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));
                windowPos.z = 1.0f;
                glm::vec3 rayFar = glm::unProject(windowPos, game->view, game->perspectiveProjection,
                                                  glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT));

                glm::vec3 rayOrigin = game->camera.position;
                glm::vec3 rayDirection = glm::normalize(rayFar - rayNear);
                glm::vec3 inverseRayDirection = 1.0f / rayDirection;

                int closestId = -1;
                float closestDistance = FLT_MAX;

                for(int entityIndex = 0; entityIndex < game->sceneEntities.size(); entityIndex++)
                {
                    Entity *entity = game->sceneEntities[entityIndex];

                    AABB transformedAABB = {glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.min, 1.0f)),
                                            glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.max, 1.0f))};

                    //glm::vec3 intersectionPoint;
                    //bool result = RayBoxIntersection(&transformedAABB, rayOrigin, rayDirection, &intersectionPoint);
                    //if(result)
                    //{
                    //    float distance = glm::length2(intersectionPoint - rayOrigin);
                    //    if(distance < closestDistance)
                    //    {
                    //        closestDistance = distance;
                    //        closestId = entity->id;
                    //    }
                    //}

                    float intersectionDist;
                    bool result = RayBoxIntersection(&transformedAABB, rayOrigin, inverseRayDirection, &intersectionDist);
                    if(result)
                    {
                        if(intersectionDist < closestDistance)
                        {
                            closestDistance = intersectionDist;
                            closestId = entity->id;
                        }
                    }
                }

                if((closestId < 0) || !game->input.keys[SDL_SCANCODE_LSHIFT])
                {
                    game->selectedIDs.clear();
                }

                if(closestId >= 0)
                {
                    uint16 pickedId = (uint16)closestId;
                    bool isAlreadyPicked = game->selectedIDs.count(pickedId);
                    if(isAlreadyPicked && game->input.keys[SDL_SCANCODE_LSHIFT])
                    {
                        game->selectedIDs.erase(pickedId);
                    }
                    else
                    {
                        game->selectedIDs.insert(pickedId);
                    }
                }

                uint64 end = SDL_GetPerformanceCounter();
                SDL_Log("%f ms", ((end - start) / (float)game->perfFreq) * 1000.0f);

                game->lastSelectedId = closestId;

                float visibleRayLength = 2000.0f;

                glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, rayOrigin, rayDirection, visibleRayLength);

                UpdateLine(&line, rayOrigin, rayOrigin + rayDirection * visibleRayLength);

                target = glm::vec2(intersectionPoint.x, intersectionPoint.z);
                targetDirection = target - glm::vec2(game->soldierEntity->position.x, game->soldierEntity->position.z);

                //Prevents silent division by zero in the glm::normalize and NaN in the targetDirection as a result
                if(glm::length2(targetDirection) > 0.00001f)
                {
                    targetDirection = glm::normalize(targetDirection);
                    targetAngle = glm::degrees(glm::atan(targetDirection.x, targetDirection.y));
                }
            }

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

            //UseShader(game->animationShader);
            //ShaderSetMatrix4Array(game->animationShader, "u_skinning", glm::value_ptr(model->animData.skinningMatrices[0]), 100);
            //RenderEntity(game->soldierEntity, game);
            //RenderModel(game, model, glm::mat4(1.0f));

            //RenderModel(game, abramsTurret, PrepareModelMatrix(tankTurret.position, tankTurret.rotation, tankTurret.scale));

            //RenderLine(&line);
            for(int i = 0; i < ArrayCount(frustumLines); i++)
            {
                RenderLine(&frustumLines[i]);
            }
            for(int i = 0; i < ArrayCount(frustumNormals); i++)
            {
                RenderLine(&frustumNormals[i]);
            }

            RenderTerrain(game);

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

            if(game->input.mouseButtons[MOUSE_LEFT] && !ImGui::GetIO().WantCaptureMouse)
            {
                selectionBoxSize = tempInputCopy.mousePos - selectionBoxStart;
                RenderRectUI(game, selectionBoxStart, selectionBoxSize, game->selectionBoxShader);
            }

            if(IsMouseJustReleased(game, MOUSE_LEFT) &&
               (glm::abs(selectionBoxSize.x) > 5 && glm::abs(selectionBoxSize.y) > 5))
            {
                glm::vec2 mouse = tempInputCopy.mousePos;

                mouse.y = WINDOW_HEIGHT - mouse.y;
                selectionBoxStart.y = WINDOW_HEIGHT - selectionBoxStart.y;

                glm::vec2 min = glm::vec2(glm::min(selectionBoxStart.x, mouse.x),
                                          glm::min(selectionBoxStart.y, mouse.y));
                glm::vec2 max = glm::vec2(glm::max(selectionBoxStart.x, mouse.x),
                                          glm::max(selectionBoxStart.y, mouse.y));

                glm::vec4 viewport = glm::vec4(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT);

                glm::vec2 positions[] = {
                    glm::vec2(min.x, min.y), //Bottom left
                    glm::vec2(max.x, min.y),
                    glm::vec2(min.x, max.y),
                    glm::vec2(max.x, max.y),
                };

                glm::vec3 nearPoints[4], farPoints[4];
                for(int i = 0; i < ArrayCount(positions); i++)
                {
                    glm::vec3 rayNear = glm::unProject(glm::vec3(positions[i], 0.0f), game->view, game->perspectiveProjection, viewport);
                    glm::vec3 rayFar = glm::unProject(glm::vec3(positions[i], 1.0f), game->view, game->perspectiveProjection, viewport);

                    glm::vec3 rayDirection = glm::normalize(rayFar - rayNear);
                    glm::vec3 rayOrigin = game->camera.position + rayDirection * 0.5f;

                    nearPoints[i] = rayOrigin;
                    farPoints[i] = rayOrigin + rayDirection * 2000.0f;

                    UpdateLine(&frustumLines[i], nearPoints[i], farPoints[i]);
                }

                glm::vec3 nBL = nearPoints[FrustumPoint_BottomLeft];
                glm::vec3 nUL = nearPoints[FrustumPoint_UpperLeft];
                glm::vec3 nBR = nearPoints[FrustumPoint_BottomRight];
                glm::vec3 nUR = nearPoints[FrustumPoint_UpperRight];

                glm::vec3 fBL = farPoints[FrustumPoint_BottomLeft];
                glm::vec3 fUL = farPoints[FrustumPoint_UpperLeft];
                glm::vec3 fBR = farPoints[FrustumPoint_BottomRight];
                glm::vec3 fUR = farPoints[FrustumPoint_UpperRight];

                //float rayLength = 2000.0f;
                float rayLength = 0.1f;

                Plane selectionPlanes[6];

                selectionPlanes[FrustumPlane_Left] = CreatePlane(nUL, nBL, fBL);
                selectionPlanes[FrustumPlane_Right] = CreatePlane(nBR, nUR, fUR);
                selectionPlanes[FrustumPlane_Up] = CreatePlane(nUR, nUL, fUL);
                selectionPlanes[FrustumPlane_Bottom] = CreatePlane(nBL, nBR, fBR);
                selectionPlanes[FrustumPlane_Near] = CreatePlane(nBL, nUL, nUR);
                selectionPlanes[FrustumPlane_Far] = CreatePlane(fUR, fUL, fBL);

                UpdateLine(&frustumNormals[FrustumPlane_Left], nUL, nUL + selectionPlanes[FrustumPlane_Left].normal * rayLength);
                UpdateLine(&frustumNormals[FrustumPlane_Right], nUR, nUR + selectionPlanes[FrustumPlane_Right].normal * rayLength);
                UpdateLine(&frustumNormals[FrustumPlane_Up], nUR, nUR + selectionPlanes[FrustumPlane_Up].normal * rayLength);
                UpdateLine(&frustumNormals[FrustumPlane_Bottom], nBL, nBL + selectionPlanes[FrustumPlane_Bottom].normal * rayLength);
                UpdateLine(&frustumNormals[FrustumPlane_Near], nBL, nBL + selectionPlanes[FrustumPlane_Near].normal * rayLength);
                UpdateLine(&frustumNormals[FrustumPlane_Far], nBR, nBR + selectionPlanes[FrustumPlane_Far].normal * rayLength);

                game->selectedIDs.clear();

                for(int entityIndex = 0; entityIndex < game->sceneEntities.size(); entityIndex++)
                {
                    Entity *entity = game->sceneEntities[entityIndex];

                    AABB transformedAABB = {glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.min, 1.0f)),
                                            glm::vec3(entity->modelMatPosScale * glm::vec4(entity->aabb.max, 1.0f))};

                    if(FrustumAABBIntersectionTest(selectionPlanes, &transformedAABB))
                    {
                        game->selectedIDs.insert(entity->id);
                    }
                }
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

