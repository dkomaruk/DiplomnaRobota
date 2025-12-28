#ifndef GAME_H

#include "entity.h"
#include "audio.h"
#include "input.h"
#include "text.h"

#include "camera.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <map>
#include <unordered_set>

#ifdef WINDOW_TRANSPARENT
    #define WINDOW_WIDTH 1920.0f
    #define WINDOW_HEIGHT 1080.0f
#else
    #define WINDOW_WIDTH 1280.0f
    #define WINDOW_HEIGHT 720.0f
#endif

struct Game
{
    SDL_Window *window;

    //Application flags
    bool isRunning = true;
    bool lockFPS;

    //Shaders and their uniforms
    std::vector<GLuint> shaders;
    GLuint mainShader, lightSourceShader, outlineShader, pickingShader, postProcessShader, uiShader;

    bool outlinePass, pickingPass;
    float outlineThickness = 2.0f;

    //Framebuffers
    //TODO: Move these framebuffers out into a struct
    GLuint pickingFbo;
    GLuint pickingTexture;

    GLuint outlineFbo;
    GLuint outlineTexture, fullSceneTexture;

    //Post-processing
    Mesh fullscreenQuad;

    //Camera
    Camera camera;
    mat4 view, perspectiveProjection, orthoProjection;

    //Input
    int prevKeys[SDL_SCANCODE_COUNT], keys[SDL_SCANCODE_COUNT];

    int mouseButtons[MOUSE_BUTTONS_COUNT], prevMouseButtons[MOUSE_BUTTONS_COUNT];
    bool isCursorHidden;

    //Timing
    float deltaTime;
    Uint64 perfFreq;
    Uint64 lastFrame;

    //Scene
    std::vector<Entity *> sceneEntities;
    std::unordered_set<uint32> selectedIDs;
    Entity *testEntity;

    //Audio
    Audio audio; //TODO: Audio API
    ALuint source;

    //Fonts
    TTF_Font *font4, *font18, *font24, *font36;
    std::map<int, TTF_Font *> fonts;

    //Game temp stuff
    Text textCounter;
    std::vector<Text> texts;
    float lastTextX = 150.0f;
    float lastTextY = 150.0f;
};

bool InitGame(Game *game);
void UpdateGame(Game *game);

void RenderScene(Game *game);

Game *GetGame();

#define GAME_H
#endif