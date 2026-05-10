#ifndef TERRAIN_H

#include "mesh.h"

#include <glm/vec3.hpp>

struct Game;
struct Ray;

struct Terrain
{
    float *heightmap;
    glm::vec2 mapSize;

    Texture heightmapTexture;
    Texture normalmapTexture;

    glm::vec2 worldSize;

    float yShift;
    float mapScale;

    GLuint shader;

    Mesh mesh;
    //Texture texture;

    Texture splatMap;

    Texture texture0;
    Texture texture1;
    Texture texture2;
    Texture texture3;
};

float *GetHeightmapData(void *image, int channels, glm::vec2 fullMapSize, glm::vec2 mapSize, float yScale, float yShift);
Terrain CreateTerrainFromImage(char *heightmapPath, float maxHeight = 6.0f, float mapPortion = 1.0f,
                      float mapScale = 0.1f, int meshStep = 1, float yShift = 0.0f);
Terrain CreateTerrainMesh(float *heightmap, glm::vec2 fullMapSize, float mapPortion = 1.0f,
                          float mapScale = 0.1f, int meshStep = 1, float yShift = 0.0f);

float GetTerrainHeight(Terrain *terrain, float x, float z);
glm::vec3 GetRayTerrainIntersection(Terrain *terrain, Ray *pickingRay, float maxDist);

void RenderTerrain(Game *game);

#define TERRAIN_H
#endif