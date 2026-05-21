#ifndef TERRAIN_EDITOR_H

#include <FastNoiseLite.h>

#include <imgui.h>
#include <glm/vec3.hpp>

struct Game;

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
    glm::vec3 center;
    float radius = 75.0f;
    float strength = 50.0f;
    i32 kernelSize = 3;
    i32 noiseType = FNL_NOISE_OPENSIMPLEX2;
    float noiseFreq = 0.5f;
    i32 octaves = 4;
};

void UpdateTerrainEditorUI(Game *game, bool *windowState, ImGuiWindowFlags flags);
void UpdateTerrainEditor(Game *game, ImGuiWindowFlags flags);

#define TERRAIN_EDITOR_H
#endif