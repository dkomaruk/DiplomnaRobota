#include "game.h"

#include "text.h"
#include "frustum.h"
#include "shader.h"
#include "camera.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <stb_image.h>
#include <stb_image_write.h>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

bool InitGame(Game *game)
{
    SDL_SetHint(SDL_HINT_TIMER_RESOLUTION, "0");

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL. Error: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

#if defined(WINDOW_TRANSPARENT) || defined(WINDOW_BORDERLESS)
    bool isBorderless = true;
    bool isTransparent = true;
#else
    bool isBorderless = false;
    bool isTransparent = false;
#endif

    Uint64 windowFlags = SDL_WINDOW_OPENGL |
                        (isBorderless ? SDL_WINDOW_TRANSPARENT : 0) |
                        (isTransparent ? SDL_WINDOW_TRANSPARENT : 0);

    game->window = SDL_CreateWindow("Komaruk Diplom", (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, windowFlags);
    if(!game->window)
    {
        SDL_Log("Failed to create a window. Error: %s", SDL_GetError());
        return false;
    }

    if(!TTF_Init())
    {
        SDL_Log("Failed to initialize SDL_ttf library. Error: %s", SDL_GetError());
        return false;
    }

    SDL_GLContext context = SDL_GL_CreateContext(game->window);
    if(!context)
    {
        SDL_Log("Failed to create an OpenGL context. Error: %s", SDL_GetError());
        return false;
    }

    GLenum glewResult = glewInit();
    if(glewResult != GLEW_OK)
    {
        SDL_Log("Failed to initialize glew. Error: %s", glewGetErrorString(glewResult));
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("../data/fonts/Roboto-Regular.ttf", 20.0f);

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(game->window, context);
    ImGui_ImplOpenGL3_Init("#version 460");

    SDL_Time ticks;
    SDL_GetCurrentTime(&ticks);
    SDL_srand(ticks);

    SDL_HideCursor();
    SDL_SetWindowRelativeMouseMode(game->window, true);
    game->input.isCursorHidden = true;

    stbi_set_flip_vertically_on_load(true);
    stbi_flip_vertically_on_write(true);

    Camera *camera = &game->camera;

    camera->position = glm::vec3(0.0f, 0.0f, 5.0f);
    camera->direction = glm::vec3(0.0f, 0.0f, -1.0f);
    camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
    //camera->speed = 2.5f;
    camera->speed = 5.0f;
    //camera->speed = 40.0f;
    camera->sensitivity = 0.1f;

    game->perspectiveProjection = glm::perspective(glm::radians(camera->fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    game->orthoProjection = glm::ortho(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, -1.0f, 1.0f);
    game->orthoProjDirLight = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f);
    //game->orthoProjDirLight = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 10.0f);

    game->view = lookAt(camera->position, camera->position + camera->direction, glm::vec3(0.0f, 1.0f, 0.0f));

    game->perfFreq = SDL_GetPerformanceFrequency();

    //SDL_GL_SetSwapInterval(1); //VSync

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_MULTISAMPLE);

    Audio *audio = &game->audio;
    audio->device = alcOpenDevice(0);
    if(!audio->device)
    {
        SDL_Log("Failed to open an OpenAL device\n");
        return false;
    }

    audio->context = alcCreateContext(audio->device, NULL);
    if(!audio->context || alcMakeContextCurrent(audio->context) == ALC_FALSE)
    {
        SDL_Log("Failed to create an OpenAL context\n");
        return false;
    }

    return true;
}

void RenderScene(Game *game)
{
    glm::vec4 bgColor = game->outlinePass ? glm::vec4(0.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i = 0; i < game->sceneEntities.size(); i++)
    {
        bool isSelected = (game->selectedIDs.find(game->sceneEntities[i]->id) != game->selectedIDs.end());
        if((game->outlinePass && isSelected) || !game->outlinePass)
        {
            Entity *e = game->sceneEntities[i];
            e->Render(e, game);
        }
    }
}

void RenderGame(Game *game)
{
    glEnable(GL_DEPTH_TEST);

    game->shadowPass = true;
    glBindFramebuffer(GL_FRAMEBUFFER, game->shadowMapFbo.id);
    glViewport(0, 0, game->shadowMapFbo.depth.x, game->shadowMapFbo.depth.y);
    RenderScene(game);
    game->shadowPass = false;

    game->outlinePass = true;
    glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);
    glViewport(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->outlineFbo.color.id, 0);
    RenderScene(game);
    game->outlinePass = false;

    glPolygonMode(GL_FRONT_AND_BACK, game->polygonMode);

    glDepthMask(GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game->fullSceneTexture.id, 0);
    ShaderSetVec4(game->lightSourceShader, "u_color", glm::vec4(1.0f));
    RenderScene(game);

//#ifdef DEBUG
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
//#endif

#ifdef LOAD_ASSETS
    if(game->renderTerrain)
    {
        RenderTerrain(game);
    }

    //Skymap
    if(game->polygonMode == GL_FILL)
    {
        UseShader(game->skymapShader);
        glDepthFunc(GL_LEQUAL);

        ShaderSetMatrix4(game->skymapShader, "u_viewProjInverse", game->projViewInverse);

        SetTexture(game->skymapTexture.id, 0);
        ShaderSetInt(game->skymapShader, "u_skyMap", 0);
        glBindVertexArray(game->fullscreenQuad.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDepthFunc(GL_LESS);
    }

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
#endif

    //Final pass, post-processing and combination of previously rendered framebuffers
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

    //UI
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(game->input.mouseButtons[MOUSE_LEFT] && RECT_HAS_SIZE(game->selectionBox.size) &&
        !game->input.isMouseCapturedByImgui)
    {
        RenderSelectionBox(game, &game->selectionBox);
    }

    if(game->renderCounters)
    {
        RenderText(&game->aliveParticlesText);
        RenderText(&game->deadParticlesText);
        RenderText(&game->fpsCounter);
        RenderText(&game->msPerFrame);
    }

    glDisable(GL_BLEND);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(game->window);
}

Game *GetGame()
{
    static Game game;
    return &game;
}

void UpdateGame(Game *game)
{
    Input *input = &game->input;

    if(input->shouldQuit)
    {
        game->isRunning = false;
    }
    if(input->keys[SDL_SCANCODE_ESCAPE])
    {
        game->isRunning = false;
        return;
    }

    UpdateCamera(game);
    game->projViewInverse = glm::inverse(game->perspectiveProjection * glm::mat4(glm::mat3(game->view)));

    static bool isPaused = false;
    if(IsFirstPress(input, SDL_SCANCODE_L))
    {
        isPaused = !isPaused;
    }
    if(isPaused)
    {
        game->deltaTime = 0.0f;
    }

    //Cast picking ray, perform selection, calculate intersection with the terrain
    if(IsFirstClick(input, MOUSE_LEFT) && !input->isMouseCapturedByImgui)
    {
        glm::vec2 mousePos = input->isCursorHidden ? WINDOW_CENTER : input->mousePos;
        game->selectionBox.start = mousePos;

        mousePos.y = (int)WINDOW_HEIGHT - mousePos.y;
        Ray pickingRay = CastPickingRay(game, mousePos);
        SelectSingleObject(game, &pickingRay);

#ifdef LOAD_ASSETS
        float visibleRayLength = 2000.0f;
        glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, &pickingRay, visibleRayLength);

//#ifdef DEBUG
        UpdateLine(&game->pickingRay, pickingRay.origin,
                   pickingRay.origin + pickingRay.direction * visibleRayLength);
//#endif

        game->target = glm::vec2(intersectionPoint.x, intersectionPoint.z);
        game->targetDirection = game->target - glm::vec2(game->soldierEntity->position.x,
                                                         game->soldierEntity->position.z);

        //Prevents silent division by zero in the glm::normalize and NaN in the targetDirection as a result
        if(glm::length2(game->targetDirection) > 0.00001f)
        {
            game->targetDirection = glm::normalize(game->targetDirection);
            game->targetAngle = glm::degrees(glm::atan(game->targetDirection.x, game->targetDirection.y));
        }
#endif
    }

    if(game->input.mouseButtons[MOUSE_LEFT] && !input->isMouseCapturedByImgui && !game->input.isCursorHidden)
    {
        game->selectionBox.size = input->mousePos - game->selectionBox.start;
    }

    if(IsMouseJustReleased(input, MOUSE_LEFT) && !input->isMouseCapturedByImgui && RECT_HAS_SIZE(game->selectionBox.size))
    {
        SelectMultipleObjects(game);
    }

    ImGuiIO &io = ImGui::GetIO();
    if(!io.WantCaptureKeyboard)
    {
        if(IsFirstPress(input, SDL_SCANCODE_1)) game->particleEditorWindow = !game->particleEditorWindow;
        if(IsFirstPress(input, SDL_SCANCODE_2)) game->terrainGeneratorWindow = !game->terrainGeneratorWindow;
        if(IsFirstPress(input, SDL_SCANCODE_3)) game->selectedEntityWindow = !game->selectedEntityWindow;
        if(IsFirstPress(input, SDL_SCANCODE_4)) game->debugSettingsWindow = !game->debugSettingsWindow;
        if(IsFirstPress(input, SDL_SCANCODE_5)) game->lightingSettingsWindow = !game->lightingSettingsWindow;
        if(IsFirstPress(input, SDL_SCANCODE_6)) game->importModelWindow = !game->importModelWindow;
        if(IsFirstPress(input, SDL_SCANCODE_7)) game->valueNoiseWindow = !game->valueNoiseWindow;
    }

    //Delete selected entities
    if(IsFirstPress(input, SDL_SCANCODE_DELETE))
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

    //Update entities
    for(int i = 0; i < game->sceneEntities.size(); i++)
    {
        Entity *entity = game->sceneEntities[i];
        UpdateEntity(game, entity);
    }

    //Test code for the test scene
#ifdef LOAD_ASSETS
    game->testEntity->rotation.y += 90.0f * game->deltaTime;
    glm::mat4 turretTransform = glm::mat4(1.0f);
    turretTransform = glm::translate(turretTransform, glm::vec3(0.0f, 0.0f, 0.25f + (sinf((float)SDL_GetTicks() / 1000.0f) + 1.0f) / 2.0f));
    turretTransform = glm::rotate(turretTransform, glm::radians(45.0f * (SDL_GetTicks() / 1000.0f)), glm::vec3(0.0f, 0.0f, 1.0f));
    Entity *tank = game->tank;
    tank->turret.transform = tank->model->nodes[tank->turret.nodeId].localTransform * turretTransform;

    glm::mat4 gunTransform = glm::mat4(1.0f);
    gunTransform = glm::rotate(gunTransform, glm::radians(sinf(SDL_GetTicks() / 500.0f) * 20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    tank->gun.transform = tank->model->nodes[tank->gun.nodeId].localTransform * gunTransform;

    UpdateTransforms(tank);

    float speed = 3.0f;
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

#if 0
    glm::mat4 tankWorldMatrix = PrepareModelMatrix(tank->position, tank->rotation, tank->scale);
    glm::mat4 tipWorldMat = tankWorldMatrix * tank->nodeTransforms[tank->gunTipId];
    game->particleSystems[0].pos = tipWorldMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 tipRotation = glm::mat3(tipWorldMat);
    game->particleSystems[0].rotation = tipRotation;
#endif

#endif

    //Audio update
    Camera *camera = &game->camera;
    glm::vec3 forward = normalize(camera->direction);

    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = normalize(cross(forward, worldUp));
    glm::vec3 up = normalize(cross(right, forward));

    ALfloat listenerOri[6] = {
        camera->direction.x, camera->direction.y, camera->direction.z,
        up.x, up.y, up.z
    };
    alListener3f(AL_POSITION, camera->position.x, camera->position.y, camera->position.z);
    alListenerfv(AL_ORIENTATION, listenerOri);

    //Change outline thickness
    if(input->keys[SDL_SCANCODE_DOWN])
    {
        game->outlineThickness -= 5.0f * game->deltaTime;
        game->outlineThickness = SDL_max(game->outlineThickness, 0.0f);

        ShaderSetInt(game->postProcessShader, "u_outlineThickness", (int)game->outlineThickness);
    }
    if(input->keys[SDL_SCANCODE_UP])
    {
        game->outlineThickness += 5.0f * game->deltaTime;
        ShaderSetInt(game->postProcessShader, "u_outlineThickness", (int)game->outlineThickness);
    }

    //Toggle wireframe mode
    if(IsFirstPress(input, SDL_SCANCODE_SPACE))
    {
        game->polygonMode = (game->polygonMode == GL_LINE) ? GL_FILL : GL_LINE;
    }

#if 0
    if(IsFirstPress(input, SDL_SCANCODE_SPACE))
    {
        int sourceState;
        alGetSourcei(game->source, AL_SOURCE_STATE, &sourceState);
        if(sourceState == AL_PLAYING)
        {
            alSourcePause(game->source);
        }
        else
        {
            alSourcePlay(game->source);
        }
    }
#endif
    //Save a screenshot of outline buffer
    if(IsFirstPress(input, SDL_SCANCODE_U))
    {
        int w = (int)WINDOW_WIDTH;
        int h = (int)WINDOW_HEIGHT;
        int bytesPerPixel = 3;

        uint8 *pixels = (uint8 *)malloc(w * h * bytesPerPixel);
        glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        stbi_write_png("test2.png", w, h, bytesPerPixel, pixels, w * bytesPerPixel);
        free(pixels);
    }

    //Enable cursor and interaction with the ui
    if(IsFirstPress(input, SDL_SCANCODE_P))
    {
        if(input->isCursorHidden)
        {
            SDL_ShowCursor();
            SDL_SetWindowRelativeMouseMode(game->window, false);
        }
        else
        {
            SDL_HideCursor();
            SDL_SetWindowRelativeMouseMode(game->window, true);
        }

        input->isCursorHidden = !input->isCursorHidden;
    }

    //Display mouse button name when it's pressed
    for(int i = 0; i < MOUSE_BUTTONS_COUNT; i++)
    {
        if(IsFirstClick(input, i))
        {
            SDL_Log("%s", GetMouseButtonName(i));
        }
    }

#ifdef LOAD_ASSETS
    //Update particles
    if(IsFirstPress(input, SDL_SCANCODE_Y))
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
#endif

    //Light
    game->dirLightView = lookAt(-game->dirLight.direction * 20.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ShaderSetMatrix4(game->shadowShader, "u_lightViewProj", game->orthoProjDirLight * game->dirLightView);
    ShaderSetMatrix4(game->skinnedShadowShader, "u_lightViewProj", game->orthoProjDirLight * game->dirLightView);
    ShaderSetMatrix4(game->terrainShader, "u_lightViewProj", game->orthoProjDirLight * game->dirLightView);

    //Update timing counters
    float ms = game->deltaTime * 1000.0f;

    char buffer[20];
    sprintf(buffer, "%.5f ms/f", ms);
    UpdateText(&game->msPerFrame, buffer);

    sprintf(buffer, "%.5f FPS", 1000.0f / ms);
    UpdateText(&game->fpsCounter, buffer);

    //Update shaders
    ShaderSetVec3(game->mainShader, "u_viewPos", game->camera.position);
    ShaderSetVec3(game->mainShader, "u_viewDir", game->camera.direction);

    ShaderSetVec3(game->animationShader, "u_viewPos", game->camera.position);
    ShaderSetVec3(game->animationShader, "u_viewDir", game->camera.direction);

    float time = SDL_GetTicks() / 1000.0f;
    ShaderSetFloat(game->mainShader, "u_time", time);
    ShaderSetFloat(game->animationShader, "u_time", time);
    ShaderSetFloat(game->postProcessShader, "u_time", time);

    ShaderSetMatrix4(game->mainShader, "u_view", game->view);
    ShaderSetMatrix4(game->animationShader, "u_view", game->view);
    ShaderSetMatrix4(game->lightSourceShader, "u_view", game->view);
    ShaderSetMatrix4(game->skinnedOutlineShader, "u_view", game->view);
    ShaderSetMatrix4(game->particleShader, "u_view", game->view);
    ShaderSetMatrix4(game->lineShader, "u_view", game->view);

    //ShaderSetMatrix4(game->mainShader, "u_projection", game->projection);

    //Reset deltas
    input->mouseDelta = glm::vec2(0.0f);
    input->mouseWheelDelta = glm::vec2(0.0f);
    input->typedText = "";
    input->isBackspacePressed = false;
}