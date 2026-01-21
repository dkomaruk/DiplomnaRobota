#ifndef TERRAIN_H

#include "mesh.h"

#include <glm/vec3.hpp>

struct Game;

struct Terrain
{
    float *heightmap;
    int mapWidth;
    int mapHeight;
    int worldWidth;
    int worldHeight;
    float yScale;
    float yShift;
    float mapScale;

    Mesh mesh;
    Texture texture;
};

Terrain CreateTerrain(char *heightmapPath, float maxHeight = 6.0f, float mapPortion = 1.0f, float mapScale = 0.1f, int meshStep = 1);

float GetTerrainHeight(Terrain *terrain, float x, float z);
glm::vec3 GetRayTerrainIntersection(Terrain *terrain, glm::vec3 rayOrigin, glm::vec3 rayDirection, float maxDist);

void RenderTerrain(Game *game);

#define TERRAIN_H
#endif