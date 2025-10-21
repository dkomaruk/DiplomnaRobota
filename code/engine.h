#ifndef ENGINE_H

#include "graphics/camera.h"

#include <SDL3/SDL.h>
#include <glm/mat4x4.hpp>

#ifdef WINDOW_TRANSPARENT
    #define WINDOW_WIDTH 1920.0f
    #define WINDOW_HEIGHT 1080.0f
#else
    #define WINDOW_WIDTH 1280.0f
    #define WINDOW_HEIGHT 720.0f
#endif

struct Engine
{
    bool isRunning = true;
    bool lockFPS;

    SDL_Window *window;

    Camera camera;
    glm::mat4 view, projection;

    int prevKeys[SDL_SCANCODE_COUNT], keys[SDL_SCANCODE_COUNT];

    float deltaTime;
    Uint64 perfFreq;
    Uint64 lastFrame;
};

bool InitEngine(Engine *engine);
void UpdateEngine(Engine *engine);

#define ENGINE_H
#endif