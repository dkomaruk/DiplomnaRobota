#include "game.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <stb_image.h>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

bool InitGame(Game *game)
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL. Error: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

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

    SDL_Time ticks;
    SDL_GetCurrentTime(&ticks);
    SDL_srand(ticks);

    SDL_HideCursor();
    SDL_SetWindowRelativeMouseMode(game->window, true);
    game->isCursorHidden = true;

    stbi_set_flip_vertically_on_load(true);
    stbi_flip_vertically_on_write(true);

    Camera *camera = &game->camera;

    camera->position = vec3(0.0f, 0.0f, 5.0f);
    camera->direction = vec3(0.0f, 0.0f, -1.0f);
    camera->up = vec3(0.0f, 1.0f, 0.0f);
    //camera->speed = 2.5f;
    camera->speed = 5.0f;
    camera->sensitivity = 0.1f;

    game->projection = perspective(radians(camera->fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    game->view = lookAt(camera->position, camera->position + camera->direction, vec3(0.0f, 1.0f, 0.0f));

    game->perfFreq = SDL_GetPerformanceFrequency();

    SDL_GL_SetSwapInterval(1); //VSync

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

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
    vec4 bgColor = game->outlinePass ? vec4(0.0f) : vec4(0.1f, 0.0f, 0.0f, 0.0f);

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
    Camera *camera = &game->camera;
    int *keyboardState = game->keys;

    vec3 dir = camera->direction;
    //dir.y = 0.0f;
    dir = normalize(dir);

    float cameraSpeed = camera->speed * game->deltaTime;
    if(keyboardState[SDL_SCANCODE_W])
        camera->position += cameraSpeed * dir;
    if(keyboardState[SDL_SCANCODE_S])
        camera->position -= cameraSpeed * dir;
    if(keyboardState[SDL_SCANCODE_A])
        camera->position -= normalize(cross(dir, camera->up)) * cameraSpeed;
    if(keyboardState[SDL_SCANCODE_D])
        camera->position += normalize(cross(dir, camera->up)) * cameraSpeed;

    //camera->position.y = 1.0f;

    game->view = lookAt(camera->position, camera->position + camera->direction, vec3(0.0f, 1.0f, 0.0f));
}

void UpdateGame(Game *game)
{
    int *keyboardState = game->keys;

    UpdateCamera(game);

    if(keyboardState[SDL_SCANCODE_ESCAPE])
    {
        game->isRunning = false;
    }

    for(int i = 0; i < game->sceneEntities.size(); i++)
    {
        Entity *entity = game->sceneEntities[i];
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
    if(IsFirstPress(game, SDL_SCANCODE_U))
    {
        int w = (int)WINDOW_WIDTH;
        int h = (int)WINDOW_HEIGHT;
        int bytesPerPixel = 3;

        uint8 *pixels = (uint8 *)malloc(w * h * bytesPerPixel);
        glBindFramebuffer(GL_FRAMEBUFFER, game->pickingFbo);
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
        float newQuadX = game->lastQuadX + game->textSize.x;
        float newQuadY = game->lastQuadY;
        if(newQuadX >= WINDOW_WIDTH)
        {
            newQuadX = 0.0f;
            newQuadY += game->textSize.y + 10.0f;
        }

        Mesh newQuad = CreateQuad(vec2(newQuadX, newQuadY), game->textSize, game->uiShader);
        newQuad.material.diffuseTexture = game->textTexture;
        game->quads.push_back(newQuad);

        game->lastQuadX = newQuadX;
        game->lastQuadY = newQuadY;
    }

    for(int i = 0; i < MOUSE_BUTTONS_COUNT; i++)
    {
        if(IsFirstClick(game, i))
        {
            SDL_Log("%s", GetMouseButtonName(i));
        }
    }

    game->testEntity->rotation.y = (float)SDL_GetTicks() / 25.0f;

    //Update shaders
    ShaderSetVec3(game->mainShader, "u_viewPos", game->camera.position);
    ShaderSetVec3(game->mainShader, "u_viewDir", game->camera.direction);

    float time = SDL_GetTicks() / 1000.0f;
    ShaderSetFloat(game->mainShader, "u_time", time);
    ShaderSetFloat(game->postProcessShader, "u_time", time);

    ShaderSetMatrix4(game->mainShader, "u_view", game->view);
    ShaderSetMatrix4(game->lightSourceShader, "u_view", game->view);
    ShaderSetMatrix4(game->pickingShader, "u_view", game->view);

    //ShaderSetMatrix4(game->mainShader, "u_projection", game->projection);
}