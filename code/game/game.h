#ifndef GAME_H

#include "entity.h"
#include "input.h"
#include "asset_manager.h"
#include "text.h"
#include "particle_system.h"
#include "editor.h"
#include "terrain.h"
#include "selection.h"
#include "ray.h"
#include "light.h"
#include "line.h"
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
    glm::ivec2 windowSize;

    Editor editor;
    AssetManager assets;

    //Application flags
    bool isRunning = true;

    //Rendering settings
    bool outlinePass;
    bool shadowPass;
    float outlineThickness = 1.0f;

    //Framebuffers
    Framebuffer outlineFbo;
    Texture fullSceneTexture, fullSceneDepthTexture;

    Framebuffer particlesFbo;

    //Shadows
    Framebuffer shadowMapFbo;
    glm::mat4 dirLightView;
    glm::mat4 orthoProjDirLight;
    glm::vec2 shadowVolumeX = glm::vec2(-50.0f, 50.0f), shadowVolumeY = glm::vec2(-50.0f, 50.0f),
              shadowVolumeZ = glm::vec2(1.0f, 50.0f);
    float shadowVolumeOffset = 5.0f;
    Line shadowVolume[12];

    //Post-processing
    Mesh fullscreenQuad;

    //Lighting
    DirectionalLight dirLight;

    //Environment
    Texture skymapTexture;

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
    std::unordered_set<u16> selectedIDs;
    int lastSelectedId = -1;

    Entity *dancingEntity;
    Entity *runningEntity;
    Entity *tank;

    Terrain terrain;
    float terrainUVMultiplier = 16.0f;

    bool measuringTerrainPerf = false;
    Timer terrainPerfTimer;
    GLuint timeQuery;
    std::vector<double> results;

    //Fonts
    std::map<int, Font> fonts;

    //Selection
    SelectionBox selectionBox;

    //Selection debug visualization
    Line pickingRay;
    Line frustumLines[4];
    Line frustumNormals[6];

    //Debug settings
    bool renderAABB = true;
    bool renderTerrain = true;
    bool renderSelectionFrustum = true;
    bool renderShadowVolume = true;
    bool renderPickingRay = true;
    bool renderCounters = true;
    GLenum polygonMode = GL_FILL;

    Texture perlinNoise;

    //Game temp stuff
    glm::vec2 target;
    glm::vec2 targetDirection;
    float targetAngle;

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

void RenderGame(Game *game);

Game *GetGame();

#define GAME_H
#endif