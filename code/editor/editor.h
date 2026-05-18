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

    float importScale = 1.0f;
    bool importModelWindow;

    //Terrain sculpting
    TerrainBrush terrainBrush;
    bool terrainSculpting = true;
};

void UpdateEditor(Game *game);

#define EDITOR_H
#endif