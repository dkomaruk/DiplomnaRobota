#ifndef TERRAIN_EDITOR_H

struct Game;

#include <imgui.h>

enum TerrainBrushType
{
    TerrainBrush_Add,
    TerrainBrush_Flatten,
    TerrainBrush_Smooth,
    TerrainBrush_Noise
};

struct TerrainBrush
{
    i32 type = TerrainBrush_Add;
    float radius = 75.0f;
    float strength = 50.0f;
    int kernelSize = 3;
};

void UpdateTerrainEditorUI(Game *game, bool *windowState, ImGuiWindowFlags flags);
void UpdateTerrainEditor(Game *game, ImGuiWindowFlags flags);

#define TERRAIN_EDITOR_H
#endif