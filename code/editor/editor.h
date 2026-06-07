#ifndef EDITOR_H

#include "terrain_editor.h"

struct Game;

struct Editor
{
    //Editor windows
    bool particleEditorWindow;
    bool terrainGeneratorWindow;
    bool selectedEntityWindow;
    bool debugSettingsWindow;
    bool lightingSettingsWindow;
    bool valueNoiseWindow;
    bool assetPlacementWindow;

    float importScale = 1.0f;
    bool importModelWindow;

    bool reloadParticles;
    std::string particleSettingsPath;

    //Terrain sculpting
    Texture perlinNoise;
    TerrainBrush terrainBrush;
    bool terrainSculpting = true;
    glm::vec2 lastMousePos;
    bool regenerateTerrain;
    std::string heightmapPath;
};

void UpdateEditor(Game *game);

#define EDITOR_H
#endif