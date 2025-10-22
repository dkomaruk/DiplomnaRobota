#include "engine.h"

#include <stb_image.h>

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

using namespace glm;

bool InitEngine(Engine *engine)
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

    engine->window = SDL_CreateWindow("Diplom", (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, windowFlags);
    if(!engine->window)
    {
        SDL_Log("Failed to create a window. Error: %s", SDL_GetError());
        return false;
    }

    SDL_GLContext context = SDL_GL_CreateContext(engine->window);
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
    SDL_SetWindowRelativeMouseMode(engine->window, true);

    stbi_set_flip_vertically_on_load(true);

    Camera *camera = &engine->camera;

    camera->position = vec3(0.0f, 0.0f, -5.0f);
    camera->direction = vec3(0.0f, 0.0f, -1.0f);
    camera->up = vec3(0.0f, 1.0f, 0.0f);
    camera->speed = 10.0f;
    camera->sensitivity = 0.1f;

    engine->projection = perspective(radians(camera->fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    engine->view = lookAt(camera->position, camera->position + camera->direction, vec3(0.0f, 1.0f, 0.0f));

    engine->perfFreq = SDL_GetPerformanceFrequency();

    //SDL_GL_SetSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    return true;
}

void UpdateCamera(Engine *engine)
{
    Camera *camera = &engine->camera;
    int *keyboardState = engine->keys;

    float cameraSpeed = camera->speed * engine->deltaTime;
    if(keyboardState[SDL_SCANCODE_W])
        camera->position += cameraSpeed * camera->direction;
    if(keyboardState[SDL_SCANCODE_S])
        camera->position -= cameraSpeed * camera->direction;
    if(keyboardState[SDL_SCANCODE_A])
        camera->position -= normalize(cross(camera->direction, camera->up)) * cameraSpeed;
    if(keyboardState[SDL_SCANCODE_D])
        camera->position += normalize(cross(camera->direction, camera->up)) * cameraSpeed;

    engine->view = lookAt(camera->position, camera->position + camera->direction, vec3(0.0f, 1.0f, 0.0f));
}

void UpdateEngine(Engine *engine)
{
    int *keyboardState = engine->keys;

    UpdateCamera(engine);

    if(keyboardState[SDL_SCANCODE_ESCAPE])
    {
        engine->isRunning = false;
    }

}