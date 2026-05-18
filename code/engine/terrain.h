#ifndef TERRAIN_H

#include "mesh.h"

#include <glm/vec3.hpp>

struct Game;
struct Ray;

struct Terrain
{
    float *heightmap;

    glm::vec2 mapSize;
    glm::vec2 worldSize;
    glm::vec2 halfWorldSize;

    float yShift;
    float mapScale;

    GLuint shader;
    Mesh mesh;

    Texture heightmapTexture;
    Texture colorTexture;
};

float *GetHeightmapData(void *image, int channels, glm::vec2 fullMapSize, glm::vec2 mapSize, float yScale, float yShift);
Terrain CreateTerrainFromImage(char *heightmapPath, float maxHeight = 6.0f, float mapPortion = 1.0f,
                      float mapScale = 0.1f, int meshStep = 1, float yShift = 0.0f);
Terrain CreateTerrainMesh(float *heightmap, glm::vec2 fullMapSize, float mapPortion = 1.0f,
                          float mapScale = 0.1f, int meshStep = 1, float yShift = 0.0f);
Terrain CreateTessellatedTerrainMesh(float *heightmap, glm::vec2 mapSize, int patchSize = 32,
                                     float mapScale = 0.1f, float yShift = 0.0f);

float GetTerrainHeight(Terrain *terrain, float x, float z);
glm::vec3 GetRayTerrainIntersection(Terrain *terrain, Ray *pickingRay, float maxDist);

void RenderTerrain(Game *game);

#define TERRAIN_H
#endif