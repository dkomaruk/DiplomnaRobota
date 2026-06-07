#include "terrain.h"

#include "defines.h"
#include "math_utils.h"

#include "game.h"
#include "shader.h"
#include "noise.h"
#include "ray.h"

#include <GL/glew.h>

#include <glm/gtx/intersect.hpp>

#include <stb_image.h>

#include <numeric>
#include <algorithm>

float *GetHeightmapData(void *image, int channels, glm::vec2 mapSize, TerrainGenerationSettings *s)
{
    float *heightmap = (float *)calloc((int)(mapSize.x * mapSize.y), sizeof(float));
    u16 *image16 = (u16 *)image;
    u8 *image8 = (u8 *)image;

    for(int i = 0; i < mapSize.y; ++i)
    {
        for(int j = 0; j < mapSize.x; ++j)
        {
            int sampleIndex = j + i * (int)mapSize.x;
            u16 sample = (channels == 1) ? image16[sampleIndex] : image8[sampleIndex * channels];
            heightmap[sampleIndex] = sample * s->yScale - s->yOffset;
        }
    }

    return heightmap;
}

//Loads a 16-bit PNG heightmap and generates a terrain mesh
Terrain CreateTerrainFromImage(char *heightmapPath, GLuint shader, TerrainGenerationSettings *s)
{
    int x, y, channels;
    stbi_info(heightmapPath, &x, &y, &channels);

    u8 *image = 0;
    u16 *image16 = 0;

    float yScale;

    if(channels == 1)
    {
        image16 = stbi_load_16(heightmapPath, &x, &y, &channels, 0);
        s->yScale = (s->maxHeight / 65535.0f);
    }
    else
    {
        image = stbi_load(heightmapPath, &x, &y, &channels, 0);
        s->yScale = (s->maxHeight / 255.0f);
    }

    glm::vec2 mapSize = glm::vec2(x, y);

    float *heightmap = GetHeightmapData(((channels == 1) ? image16 : (void *)image), channels, mapSize, s);
    stbi_image_free(((channels == 1) ? image16 : (void *)image));

    return CreateTessellatedTerrainMesh(heightmap, mapSize, shader, s);
}

//Create a flat mesh with the resolution of patchSize x patchSize then dynamically subdivide it using
//OpenGL tessellation and sample height of each vertex in the tessellation evaluation shader
Terrain CreateTessellatedTerrainMesh(float *heightmap, glm::vec2 mapSize, GLuint shader, TerrainGenerationSettings *s)
{
    Terrain t = {};
    t.settings = *s;
    t.shader = shader;

    t.heightmap = heightmap;
    t.heightmapTexture = CreateGLTexture(heightmap, (int)mapSize.x, (int)mapSize.y,
                                         TextureFlag_Heightmap | TextureFlag_Filter_Min_Linear |
                                         TextureFlag_Filter_Mag_Linear | TextureFlag_ClampToEdge);
    t.mapSize = mapSize;
    t.worldSize = (t.mapSize - 1.0f) * s->mapScale;
    t.halfWorldSize = t.worldSize / 2.0f;

    glm::vec2 center = t.worldSize / 2.0f;

    int patchSizeX, patchSizeY;
    patchSizeX = patchSizeY = s->patchSize;
    if(t.mapSize.y > t.mapSize.x) patchSizeY = s->patchSize * (int)(t.mapSize.y / t.mapSize.x);
    else if(t.mapSize.x > t.mapSize.y) patchSizeX = s->patchSize * (int)(t.mapSize.x / t.mapSize.y);

    int numOfVerticesX = 0;
    int numOfVerticesZ = 0;
    std::vector<TerrainVertex> vertices;

    for(int y = 0; y < patchSizeY; ++y)
    {
        numOfVerticesX++;
        numOfVerticesZ = 0;

        float v = (float)y / (patchSizeY - 1);
        float posZ = (t.worldSize.y * v) - center.y;

        for(int x = 0; x < patchSizeX; ++x)
        {
            TerrainVertex vertex = {};

            float u = (float)x / (patchSizeX - 1);
            float posX = (t.worldSize.x * u) - center.x;

            vertex.position = glm::vec3(posX, 0.0f, posZ);
            vertex.uv = glm::vec2(u, v);

            vertices.push_back(vertex);
            numOfVerticesZ++;
        }
    }

    std::vector<u32> indices;
    for (int i = 0; i < numOfVerticesX - 1; ++i)
    {
        for (int j = 0; j < numOfVerticesZ - 1; ++j)
        {
            u32 BL = j + (numOfVerticesZ * (i + 1));
            u32 BR = j + (numOfVerticesZ * i);
            u32 TR = (j + 1) + (numOfVerticesZ * i);
            u32 TL = (j + 1) + (numOfVerticesZ * (i + 1));

            indices.insert(indices.end(), {BL, BR, TR, TL});
        }
    }

    t.mesh = CreateMesh(&vertices[0], vertices.size(), terrainVertexAttribs[0].stride, &indices[0],
                        indices.size(), terrainVertexAttribs, ArrayCount(terrainVertexAttribs));
    t.mesh.drawMode = GL_PATCHES;

    return t;
}

float *GenerateTerrainHeightmap(float *noise, glm::vec2 size, TerrainGenerationSettings *s)
{
    float *heightmap = (float *)calloc((int)(size.x * size.y), sizeof(float));
    for(int i = 0; i < size.y; ++i)
    {
        for(int j = 0; j < size.x; ++j)
        {
            int sampleIndex = j + i * (int)size.x;
            int destIndex = j + i * (int)size.x;

            heightmap[destIndex] = noise[sampleIndex] * s->maxHeight - s->yOffset;
        }
    }

    return heightmap;
}

void DeleteTerrain(Terrain *terrain)
{
    DeleteMesh(&terrain->mesh);
    glDeleteTextures(1, &terrain->heightmapTexture.id);
    free(terrain->heightmap);
}

float GetTerrainHeight(Terrain *terrain, float x, float z)
{
    glm::vec2 worldPos = glm::vec2(x, z) + terrain->halfWorldSize;

    glm::vec2 mapPos = (worldPos / terrain->worldSize) * terrain->mapSize;
    mapPos = glm::clamp(mapPos, glm::vec2(0.0f), terrain->mapSize - 1.001f);

    glm::ivec2 mapPos0 = glm::ivec2(mapPos);
    glm::vec2 weights = glm::fract(mapPos);

    float h00 = terrain->heightmap[mapPos0.s + (int)terrain->mapSize.x * mapPos0.t];
    float h10 = terrain->heightmap[(mapPos0.s + 1) + (int)terrain->mapSize.x * mapPos0.t];
    float h01 = terrain->heightmap[mapPos0.s + (int)terrain->mapSize.x * (mapPos0.t + 1)];
    float h11 = terrain->heightmap[(mapPos0.s + 1) + (int)terrain->mapSize.x * (mapPos0.t + 1)];

    return glm::mix(glm::mix(h00, h10, weights.x), glm::mix(h01, h11, weights.x), weights.y);
}

//Approximate intersection of a ray with the terrain mesh which is much cheaper
//than testing the intersection of each triangle of the terrain mesh with the ray
glm::vec3 FindApproximateIntersectionPoint(Terrain *terrain, glm::vec3 origin, glm::vec3 dir,
                                           float low, float high, int iterations)
{
    for(int i = 0; i < iterations; i++)
    {
        float mid = (low + high) * 0.5f;
        glm::vec3 p = origin + dir * mid;

        float h = GetTerrainHeight(terrain, p.x, p.z);
        if (p.y < h)
            high = mid;
        else
            low = mid;
    }

    return origin + dir * ((low + high) * 0.5f);
}

glm::vec3 GetRayTerrainIntersection(Terrain *terrain, Ray *pickingRay, float maxDist)
{
    glm::vec3 result = glm::vec3(0.0f);

    if(pickingRay->origin.y < GetTerrainHeight(terrain, pickingRay->origin.x, pickingRay->origin.z))
    {
        return result;
    }

    float t = 0.0;
    float prevT = 0.0f;

    while(t < maxDist)
    {
        glm::vec3 p = pickingRay->origin + pickingRay->direction * t;

        float h = GetTerrainHeight(terrain, p.x, p.z);
        if(p.y < h)
        {
            //if the next point is below the heightmap, we passed the intersection point
            //and have to search between prevT and t to find the point which is
            //approximately close to the exact spot of the intersection
            return FindApproximateIntersectionPoint(terrain, pickingRay->origin, pickingRay->direction, prevT, t, 4);
        }

        prevT = t;
        t += terrain->settings.mapScale; //TODO: Why is map scale used as a step distance?
    }

    return result;
}

void RenderTerrain(Game *game)
{
    Terrain *terrain = &game->terrain;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glUseProgram(terrain->shader);

    ShaderSetMatrix4(terrain->shader, "u_view", game->view);

    SetTexture(terrain->heightmapTexture.id, 0);
    ShaderSetInt(terrain->shader, "u_heightmap", 0);

    SetTexture(terrain->colorTexture.id, 1);
    ShaderSetInt(terrain->shader, "u_color", 1);

    SetTexture(game->perlinNoise.id, 2);
    ShaderSetInt(terrain->shader, "u_noiseMap", 2);

    SetTexture(game->shadowMapFbo.depth.id, 3);
    ShaderSetInt(terrain->shader, "u_shadowMap", 3);

    ShaderSetFloat(terrain->shader, "u_texCoordsMultiplier", game->terrainUVMultiplier);
    ShaderSetFloat(terrain->shader, "u_mapScale", terrain->settings.mapScale);

    ShaderSetFloat(terrain->shader, "u_minDist", terrain->minTessDist);
    ShaderSetFloat(terrain->shader, "u_maxDist", terrain->maxTessDist);
    ShaderSetFloat(terrain->shader, "u_minTessLevel", terrain->minTessLevel);
    ShaderSetFloat(terrain->shader, "u_maxTessLevel", terrain->maxTessLevel);

    bool displayBrush = !game->input.isMouseCapturedByImgui &&
                        game->editor.terrainSculpting &&
                        game->editor.terrainGeneratorWindow;
    ShaderSetInt(terrain->shader, "u_sculptingMode", displayBrush);
    ShaderSetVec3(terrain->shader, "u_brushCenter", game->editor.terrainBrush.center);

    float brushRadius = game->editor.terrainBrush.radius * (terrain->worldSize.x / (terrain->mapSize.y - 1.0f));
    ShaderSetFloat(terrain->shader, "u_brushRadius", brushRadius);

    glBindVertexArray(terrain->mesh.vao);

    if(terrain->mesh.drawMode == GL_PATCHES)
    {
        glPatchParameteri(GL_PATCH_VERTICES, 4);
    }

    if(game->measuringTerrainPerf)
    {
        glBeginQuery(GL_TIME_ELAPSED, game->timeQuery);
    }

    glDrawElements(terrain->mesh.drawMode, terrain->mesh.indicesCount, GL_UNSIGNED_INT, 0);

    if(game->measuringTerrainPerf)
    {
        glEndQuery(GL_TIME_ELAPSED);
    }

    glDisable(GL_CULL_FACE);
}
