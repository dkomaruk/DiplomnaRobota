#include "game.h"

#include "text.h"
#include "infantry.h"
#include "shader.h"

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

#ifdef WINDOW_TRANSPARENT
    bool isBorderless = true;
    bool isTransparent = true;
#else
    bool isBorderless = false;
    bool isTransparent = false;
#endif

    Uint64 windowFlags = SDL_WINDOW_OPENGL |
                        (isBorderless ? SDL_WINDOW_TRANSPARENT : 0) |
                        (isTransparent ? SDL_WINDOW_TRANSPARENT : 0);

    game->window = SDL_CreateWindow("Diplom", (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, windowFlags);
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;

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

Game *GetGame()
{
    static Game game;
    return &game;
}

void UpdateCamera(Game *game)
{
    Input *input = &game->input;
    Camera *camera = &game->camera;

    glm::vec3 dir = camera->direction;
    //dir.y = 0.0f;
    dir = normalize(dir);

    float cameraSpeed = camera->speed * game->deltaTime;
    if(input->keys[SDL_SCANCODE_LSHIFT])
    {
        cameraSpeed *= 5.0f;
    }

    //Camera orientation
    if(input->isCursorHidden)
    {
        camera->yaw += input->mouseDelta.x * camera->sensitivity;
        camera->pitch -= input->mouseDelta.y * camera->sensitivity;
        camera->pitch = SDL_clamp(camera->pitch, camera->maxPitch.x, camera->maxPitch.y);

        camera->direction.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        camera->direction.y = sin(glm::radians(camera->pitch));
        camera->direction.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        camera->direction = normalize(camera->direction);
    }

    //Camera zoom
    camera->fov -= input->mouseWheelDelta.y;
    camera->fov = SDL_clamp(camera->fov, 1.0f, 45.0f);

    game->perspectiveProjection = glm::perspective(glm::radians(camera->fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);

    //Camera movement
    if(input->keys[SDL_SCANCODE_W])
        camera->position += cameraSpeed * dir;
    if(input->keys[SDL_SCANCODE_S])
        camera->position -= cameraSpeed * dir;
    if(input->keys[SDL_SCANCODE_A])
        camera->position -= normalize(cross(dir, camera->up)) * cameraSpeed;
    if(input->keys[SDL_SCANCODE_D])
        camera->position += normalize(cross(dir, camera->up)) * cameraSpeed;

    //camera->position.y = 1.0f;

    game->view = lookAt(camera->position, camera->position + camera->direction, glm::vec3(0.0f, 1.0f, 0.0f));
}

void UpdateGame(Game *game)
{
    if(game->input.shouldQuit)
    {
        game->isRunning = false;
    }

    UpdateCamera(game);

    static bool isPaused = false;
    if(IsFirstPress(game, SDL_SCANCODE_L))
    {
        isPaused = !isPaused;
    }
    if(isPaused)
    {
        game->deltaTime = 0.0f;
    }

    Input *input = &game->input;

    if(input->keys[SDL_SCANCODE_ESCAPE])
    {
        game->isRunning = false;
        return;
    }

    for(int i = 0; i < game->sceneEntities.size(); i++)
    {
        Entity *entity = game->sceneEntities[i];
        UpdateEntity(game, entity);

        switch(entity->type)
        {
            case EntityType_Infantry:
            {
                InfantrySquad *squad = (InfantrySquad *)entity;
                //squad->position.x = sin(SDL_GetTicks() / 1000.0f) * ((i % 2 == 0) ? -1 : 1);
            } break;

            case EntityType_Static:
            {

            } break;

            default:
            {
                *(int *)0 = 0; //Invalid entity type
            } break;
        }

    }

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


    if(!game->textDemoEnabled)
    {
#if 0
        if(IsFirstPress(game, SDL_SCANCODE_SPACE))
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
        if(IsFirstPress(game, SDL_SCANCODE_U))
        {
            int w = (int)WINDOW_WIDTH;
            int h = (int)WINDOW_HEIGHT;
            int bytesPerPixel = 3;

            uint8 *pixels = (uint8 *)malloc(w * h * bytesPerPixel);
            glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo.id);
            glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
            stbi_write_png("test2.png", w, h, bytesPerPixel, pixels, w * bytesPerPixel);
            free(pixels);
        }
        if(IsFirstPress(game, SDL_SCANCODE_P))
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
    }


    for(int i = 0; i < MOUSE_BUTTONS_COUNT; i++)
    {
        if(IsFirstClick(game, i))
        {
            SDL_Log("%s", GetMouseButtonName(i));
        }
    }

#ifdef LOAD_ASSETS
    game->testEntity->rotation.y += 90.0f * game->deltaTime;
#endif

    if(IsFirstPress(game, SDL_SCANCODE_T))
    {
        game->textDemoEnabled = true;
    }

    if(game->textDemoEnabled)
    {
        UpdateTextDemo(game);
    }

    //UPDATE PARTICLES
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
    ShaderSetMatrix4(game->pickingShader, "u_view", game->view);
    ShaderSetMatrix4(game->skinnedPickingShader, "u_view", game->view);
    ShaderSetMatrix4(game->particleShader, "u_view", game->view);
    ShaderSetMatrix4(game->lineShader, "u_view", game->view);

    //ShaderSetMatrix4(game->mainShader, "u_projection", game->projection);


    input->mouseDelta = glm::vec2(0.0f);
    input->mouseWheelDelta = glm::vec2(0.0f);
    input->typedText = "";
    input->isBackspacePressed = false;
}