#ifndef GAME_H

#include "entity.h"
#include "audio.h"
#include "input.h"
#include "text.h"
#include "text_demo.h"
#include "particle_system.h"
#include "terrain.h"
#include "framebuffer.h"

#include "defines.h"

#include "camera.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <map>
#include <unordered_set>

struct Game
{
    SDL_Window *window;

    //Application flags
    bool isRunning = true;
    bool lockFPS = false;

    //Shaders and their uniforms
    std::vector<GLuint> shaders; //Array of all shaders to update common uniforms in one loop
    GLuint mainShader, lightSourceShader, skinnedOutlineShader, outlineShader, pickingShader, skinnedPickingShader,
           postProcessShader, uiTextShader, particleShader, terrainShader, animationShader, lineShader,
           selectionBoxShader, aabbShader;

    bool outlinePass, pickingPass;
    float outlineThickness = 2.0f;

    //Framebuffers
    //TODO: Move these framebuffers out into a struct
    Framebuffer pickingFbo;

    Framebuffer outlineFbo;
    Texture fullSceneTexture, fullSceneDepthTexture;

    Framebuffer smokeFbo;

    //Post-processing
    Mesh fullscreenQuad;

    //Camera
    Camera camera;
    glm::mat4 view, perspectiveProjection, orthoProjection, projViewInverse;

    //Input
    Input input;

    //Timing
    float deltaTime;
    Uint64 perfFreq;
    Uint64 lastFrame;

    Text fpsCounter;
    Text msPerFrame;

    //Scene
    std::vector<Entity *> sceneEntities;
    std::unordered_set<uint16> selectedIDs;
    int lastSelectedId = -1;

    Entity *testEntity;
    Entity *soldierEntity;
    Entity *soldierEntity0;
    Entity *cubeEntity;

    Terrain terrain;

    //Audio
    Audio audio; //TODO: Audio API
    ALuint source;

    //Fonts
    std::map<int, Font> fonts;

    //Debug
    bool renderAABB = true;

    //Game temp stuff
    bool textDemoEnabled;
    TextDemo textDemo;

    //Particle system
    bool renderParticles = true;
    Text aliveParticlesText, deadParticlesText;

    ParticleSystem particleSystems[1];
    ParticleSystemSettings smokeSettings;
    ParticleData *particleData;

    int aliveParticles;
    Mesh particlesQuad;
    GLuint textureID;
    GLuint vboInstances;

    Atlas atlas;

    Texture particleTextures[13];
    int currentTexture = 1;
};

bool InitGame(Game *game);
void UpdateGame(Game *game);

void RenderScene(Game *game);

Game *GetGame();

#define GAME_H
#endif