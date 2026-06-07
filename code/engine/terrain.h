#ifndef TERRAIN_H

#include "mesh.h"

#include <glm/vec3.hpp>

struct Game;
struct Ray;

struct TerrainGenerationSettings
{
    float maxHeight = 6.0f;
    float yOffset = 0.0f;
    float yScale = 1.0f;
    float mapScale = 0.1f;
    int patchSize = 16;

    glm::ivec2 gridSize = glm::ivec2(3);
    int octaves = 3;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
};

struct Terrain
{
    float *heightmap;
    glm::vec2 mapSize, worldSize, halfWorldSize;

    Texture heightmapTexture, colorTexture;
    GLuint shader;
    Mesh mesh;

    float minTessDist = 2.0f, maxTessDist = 100.0f;
    float minTessLevel = 1.0f, maxTessLevel = 32.0f;

    TerrainGenerationSettings settings;
};

float *GenerateTerrainHeightmap(float *noise, glm::vec2 size, TerrainGenerationSettings *s);

float *GetHeightmapData(void *image, int channels, glm::vec2 mapSize, TerrainGenerationSettings *settings);
Terrain CreateTerrainFromImage(char *heightmapPath, GLuint shader = 0, TerrainGenerationSettings *settings = NULL);
Terrain CreateTessellatedTerrainMesh(float *heightmap, glm::vec2 mapSize, GLuint shader = 0,
                                     TerrainGenerationSettings *settings = 0);

void DeleteTerrain(Terrain *terrain);

float GetTerrainHeight(Terrain *terrain, float x, float z);
glm::vec3 GetRayTerrainIntersection(Terrain *terrain, Ray *pickingRay, float maxDist);

void RenderTerrain(Game *game);

#define TERRAIN_H
#endif