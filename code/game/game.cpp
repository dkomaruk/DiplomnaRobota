#include "game.h"

#include "text.h"
#include "frustum.h"
#include "renderer.h"
#include "debug.h"
#include "shader.h"
#include "camera.h"
#include "asset_manager.h"

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

#include <time.h>

Game *GetGame()
{
    static Game game;
    return &game;
}

bool InitGame(Game *game)
{
    srand((u32)time(0));

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


    Uint64 windowFlags = SDL_WINDOW_OPENGL;
#ifdef WINDOW_FULLSCREEN
    windowFlags |= SDL_WINDOW_BORDERLESS;

    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(displayID);
    game->windowSize = glm::ivec2(displayMode->w, displayMode->h);

    game->window = SDL_CreateWindow("Komaruk Diplomna Robota", game->windowSize.x, game->windowSize.y, windowFlags);
    SDL_SetWindowFullscreen(game->window, true);
#else
    game->windowSize = glm::ivec2(1280, 720);
    game->window = SDL_CreateWindow("Komaruk Diplom Robota", game->windowSize.x, game->windowSize.y, windowFlags);
#endif




    if(!game->window)
    {
        SDL_Log("Failed to create a window. Error: %s", SDL_GetError());
        return false;
    }

    SDL_GetWindowSize(game->window, &game->windowSize.x, &game->windowSize.y);

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

    ImGuiIO &io = ImGui::GetIO();
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

    game->camera = CreateFPSCamera();

    game->perspectiveProjection = glm::perspective(glm::radians(game->camera.fov), RECT_ASPECT_RATIO(game->windowSize), 0.1f, 1000.0f);
    game->orthoProjection = glm::ortho(0.0f, (float)game->windowSize.x, (float)game->windowSize.y, 0.0f, -1.0f, 1.0f);
    game->orthoProjDirLight = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f);

    game->view = glm::lookAt(game->camera.position, game->camera.position + game->camera.direction,
                             glm::vec3(0.0f, 1.0f, 0.0f));

    game->perfFreq = SDL_GetPerformanceFrequency();

    //SDL_GL_SetSwapInterval(1); //VSync

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_MULTISAMPLE);

    SetupFramebuffers(game);

    return true;
}

void RenderGame(Game *game)
{
    RenderShadowPass(game);
    RenderOutlinePass(game);
    RenderMainPass(game);

    if(game->renderParticles)
        RenderParticlePass(game);

    glDisable(GL_DEPTH_TEST);
    RenderPostProcessing(game);
    RenderUI(game);
    glEnable(GL_DEPTH_TEST);

    //Display rendered frame
    SDL_GL_SwapWindow(game->window);
}

//Test code for the demo scene
void UpdateTestScene(Game *game)
{
    glm::mat4 turretTransform = glm::mat4(1.0f);
    turretTransform = glm::translate(turretTransform, glm::vec3(0.0f, 0.0f,
                                     0.25f + (sinf((float)SDL_GetTicks() / 1000.0f) + 1.0f) / 2.0f));
    turretTransform = glm::rotate(turretTransform, glm::radians(45.0f * (SDL_GetTicks() / 1000.0f)),
                                  glm::vec3(0.0f, 0.0f, 1.0f));

    Entity *tank = game->tank;
    tank->turret.transform = tank->model->nodes[tank->turret.nodeId].localTransform * turretTransform;

    glm::mat4 gunTransform = glm::mat4(1.0f);
    gunTransform = glm::rotate(gunTransform, glm::radians(sinf(SDL_GetTicks() / 500.0f) * 20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    tank->gun.transform = tank->model->nodes[tank->gun.nodeId].localTransform * gunTransform;

    UpdateTransforms(tank);

    float speed = 3.0f;
    float x = game->runningEntity->position.x + game->targetDirection.x * speed * game->deltaTime;
    float z = game->runningEntity->position.z + game->targetDirection.y * speed * game->deltaTime;
    float y = GetTerrainHeight(&game->terrain, x, z);

    game->runningEntity->position = glm::vec3(x, y, z);

    float angleDiff = game->targetAngle - game->runningEntity->rotation.y;

    if(angleDiff < -180.0f)
        angleDiff += 360.0f;
    if(angleDiff > 180.0f)
        angleDiff -= 360.0f;

    float rotationStep = 200.0f * game->deltaTime;

    if(glm::abs(angleDiff) <= rotationStep)
        game->runningEntity->rotation.y = game->targetAngle;
    else
        game->runningEntity->rotation.y += glm::sign(angleDiff) * rotationStep;

    if(glm::distance(glm::vec2(x, z), game->target) < 0.1f)
    {
        game->targetDirection = glm::vec2(0, 0);
    }

#if 0
    glm::mat4 tankWorldMatrix = PrepareModelMatrix(tank->position, tank->rotation, tank->scale);
    glm::mat4 tipWorldMat = tankWorldMatrix * tank->nodeTransforms[tank->gunTip.nodeId];
    game->particleSystems[0].pos = tipWorldMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 tipRotation = glm::mat3(tipWorldMat);
    game->particleSystems[0].rotation = tipRotation;
#endif
}

void UpdateGame(Game *game)
{
    UpdateFPSCamera(game);

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
        glm::vec2 mousePos = input->isCursorHidden ? RECT_HALF(game->windowSize) : input->mousePos;
        game->selectionBox.start = mousePos;

        mousePos.y = (int)game->windowSize.y - mousePos.y;
        Ray pickingRay = CastPickingRay(game, mousePos);
        SelectSingleObject(game, &pickingRay);

        float visibleRayLength = 2000.0f;
        glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, &pickingRay, visibleRayLength);

        UpdateLine(&game->pickingRay, pickingRay.origin, pickingRay.origin + pickingRay.direction * visibleRayLength);

        game->target = glm::vec2(intersectionPoint.x, intersectionPoint.z);
        game->targetDirection = game->target - glm::vec2(game->runningEntity->position.x,
                                                         game->runningEntity->position.z);

        if(glm::length2(game->targetDirection) > 0.00001f)
        {
            game->targetDirection = glm::normalize(game->targetDirection);
            game->targetAngle = glm::degrees(glm::atan(game->targetDirection.x, game->targetDirection.y));
        }
    }

    if(game->input.mouseButtons[MOUSE_LEFT] && !input->isMouseCapturedByImgui && !game->input.isCursorHidden)
    {
        game->selectionBox.size = input->mousePos - game->selectionBox.start;
    }

    if(IsMouseJustReleased(input, MOUSE_LEFT) && !input->isMouseCapturedByImgui && !input->isCursorHidden &&
       RECT_HAS_SIZE(game->selectionBox.size))
    {
        SelectMultipleObjects(game);
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

    UpdateTestScene(game);

    //Change outline thickness
    if(input->keys[SDL_SCANCODE_DOWN])
    {
        game->outlineThickness -= 5.0f * game->deltaTime;
        game->outlineThickness = SDL_max(game->outlineThickness, 0.0f);

        ShaderSetInt(GetShader(game, "post_process"), "u_outlineThickness", (int)game->outlineThickness);
    }
    if(input->keys[SDL_SCANCODE_UP])
    {
        game->outlineThickness += 5.0f * game->deltaTime;
        ShaderSetInt(GetShader(game, "post_process"), "u_outlineThickness", (int)game->outlineThickness);
    }

    //Toggle wireframe mode
    if(IsFirstPress(input, SDL_SCANCODE_SPACE))
    {
        game->polygonMode = (game->polygonMode == GL_LINE) ? GL_FILL : GL_LINE;
    }

    //Save a screenshot of outline buffer
    if(IsFirstPress(input, SDL_SCANCODE_U))
    {
        SaveFramebufferContents(game->outlineFbo);
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

    //Update particles
    if(game->renderParticles)
    {
        for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
        {
            SpawnParticles(game, &game->particleSystems[i]);
            UpdateParticles(game, &game->particleSystems[i]);
        }

        SortAllParticles(game);
    }

    //Update onscreen counters
    float ms = game->deltaTime * 1000.0f;

    char buffer[20];
    sprintf(buffer, "%.5f ms/f", ms);
    UpdateText(&game->msPerFrame, buffer);

    sprintf(buffer, "%.5f FPS", 1000.0f / ms);
    UpdateText(&game->fpsCounter, buffer);

    //Light
    game->dirLightView = lookAt(-game->dirLight.direction * 20.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //game->dirLightView = lookAt(-game->dirLight.direction * 1000.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightViewProj = game->orthoProjDirLight * game->dirLightView;
    ShaderSetMatrix4(GetShader(game, "shadow"), "u_lightViewProj", lightViewProj);
    ShaderSetMatrix4(GetShader(game, "skinned_shadow"), "u_lightViewProj", lightViewProj);
    ShaderSetMatrix4(GetShader(game, "terrain"), "u_lightViewProj", lightViewProj);
    ShaderSetMatrix4(GetShader(game, "tessellated_terrain"), "u_lightViewProj", lightViewProj);

    //Update shaders
    ShaderSetVec3(GetShader(game, "main"), "u_viewPos", game->camera.position);
    ShaderSetVec3(GetShader(game, "main"), "u_viewDir", game->camera.direction);

    ShaderSetVec3(GetShader(game, "animation"), "u_viewPos", game->camera.position);
    ShaderSetVec3(GetShader(game, "animation"), "u_viewDir", game->camera.direction);

    float time = SDL_GetTicks() / 1000.0f;
    ShaderSetFloat(GetShader(game, "main"), "u_time", time);
    ShaderSetFloat(GetShader(game, "animation"), "u_time", time);
    ShaderSetFloat(GetShader(game, "post_process"), "u_time", time);

    ShaderSetMatrix4(GetShader(game, "main"), "u_view", game->view);
    ShaderSetMatrix4(GetShader(game, "animation"), "u_view", game->view);
    ShaderSetMatrix4(GetShader(game, "light_source"), "u_view", game->view);
    ShaderSetMatrix4(GetShader(game, "outline"), "u_view", game->view);
    ShaderSetMatrix4(GetShader(game, "skinned_outline"), "u_view", game->view);
    ShaderSetMatrix4(GetShader(game, "particle"), "u_view", game->view);
    ShaderSetMatrix4(GetShader(game, "grass"), "u_view", game->view);
    ShaderSetMatrix4(GetShader(game, "line"), "u_view", game->view);

    //ShaderSetMatrix4(GetShader(game, "main"), "u_projection", game->projection);

    //Reset input deltas
    input->mouseDelta = glm::vec2(0.0f);
    input->mouseWheelDelta = glm::vec2(0.0f);
    input->typedText = "";
    input->isBackspacePressed = false;
}