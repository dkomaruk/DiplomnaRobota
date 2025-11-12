#ifndef GAME_H

#include "entity.h"
#include "audio.h"

#include "graphics/camera.h"

#include <SDL3/SDL.h>
#include <glm/mat4x4.hpp>

#include <vector>

#ifdef WINDOW_TRANSPARENT
    #define WINDOW_WIDTH 1920.0f
    #define WINDOW_HEIGHT 1080.0f
#else
    #define WINDOW_WIDTH 1280.0f
    #define WINDOW_HEIGHT 720.0f
#endif

struct Game
{
    bool isRunning = true;
    bool lockFPS;

    SDL_Window *window;

    std::vector<GLuint> shaders;
    GLuint outlineShader;
    bool outlinePass;

    Camera camera;
    mat4 view, projection;

    int prevKeys[SDL_SCANCODE_COUNT], keys[SDL_SCANCODE_COUNT];

    float deltaTime;
    Uint64 perfFreq;
    Uint64 lastFrame;

    std::vector<Entity *> sceneEntities; //TODO: Get rid of std::vector

    Audio audio;
};

bool InitGame(Game *game);
void UpdateGame(Game *game);

void RenderScene(Game *game);

Game *GetGame();

#define GAME_H
#endif