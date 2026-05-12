#include "terrain.h"

#include "defines.h"
#include "math_utils.h"

#include "game.h"
#include "shader.h"
#include "ray.h"

#include <GL/glew.h>

#include <glm/gtx/intersect.hpp>

#include <stb_image.h>

float *GetHeightmapData(void *image, int channels, glm::vec2 fullMapSize, glm::vec2 mapSize, float yScale, float yShift)
{
    float *heightmap = (float *)calloc((int)(mapSize.x * mapSize.y), sizeof(float));

    u16 *image16 = (u16 *)image;
    u8 *image8 = (u8 *)image;

    for(int i = 0; i < mapSize.y; ++i)
    {
        for(int j = 0; j < mapSize.x; ++j)
        {
            int sampleIndex = j + i * (int)fullMapSize.x;
            int destIndex = j + i * (int)mapSize.x;

            u16 sample = (channels == 1) ? image16[sampleIndex] : image8[sampleIndex * channels];
            heightmap[destIndex] = sample * yScale - yShift;
        }
    }

    return heightmap;
}

//Loads a 16-bit PNG heightmap and generates a terrain mesh
Terrain CreateTerrainFromImage(char *heightmapPath, float maxHeight, float mapPortion,
                               float mapScale, int meshStep, float yShift)
{
    int x, y, channels;
    stbi_info(heightmapPath, &x, &y, &channels);

    u8 *image = 0;
    u16 *image16 = 0;

    float yScale;

    if(channels == 1)
    {
        image16 = stbi_load_16(heightmapPath, &x, &y, &channels, 0);
        yScale = (maxHeight / 65535.0f);
    }
    else
    {
        image = stbi_load(heightmapPath, &x, &y, &channels, 0);
        yScale = (maxHeight / 255.0f);
    }

    glm::vec2 fullMapSize = glm::vec2(x, y);
    glm::vec2 mapSize = fullMapSize * mapPortion;

    float *heightmap = GetHeightmapData(((channels == 1) ? image16 : (void *)image), channels, fullMapSize, mapSize, yScale, yShift);

    stbi_image_free(((channels == 1) ? image16 : (void *)image));

    //return CreateTerrainMesh(heightmap, fullMapSize, mapPortion, mapScale, meshStep, yShift);
    return CreateTessellatedTerrainMesh(heightmap, fullMapSize, mapPortion, mapScale, meshStep, yShift);
}

Terrain CreateTerrainMesh(float *heightmap, glm::vec2 fullMapSize, float mapPortion,
                          float mapScale, int meshStep, float yShift)
{
    Terrain t = {};

    int textureFlags = TextureFlag_Heightmap | TextureFlag_Filter_Min_Linear |
                       TextureFlag_Filter_Mag_Linear | TextureFlag_ClampToEdge;
    t.heightmapTexture = CreateGLTexture(heightmap, (int)fullMapSize.x, (int)fullMapSize.y, textureFlags);

    t.mapSize = fullMapSize * mapPortion;
    t.yShift = yShift;
    t.heightmap = heightmap;

    std::vector<TerrainVertex> vertices;

    int numOfVerticesX = 0;
    int numOfVerticesZ = 0;

    t.mapScale = mapScale;
    t.worldSize = (t.mapSize - 1.0f) * t.mapScale;

    glm::vec2 center = t.worldSize / 2.0f;

    for(int y = 0; y < t.mapSize.y; y += meshStep)
    {
        numOfVerticesX++;
        numOfVerticesZ = 0;

        float posX = ((float)y * t.mapScale) - center.x;

        for(int x = 0; x < t.mapSize.x; x += meshStep)
        {
            TerrainVertex vertex = {};

            float posZ = (x * t.mapScale) - center.y;
            vertex.position = glm::vec3(posX, 0.0f, posZ);

            vertex.uv = glm::vec2(x, y) / (t.mapSize - 1.0f);

            vertices.push_back(vertex);
            numOfVerticesZ++;
        }
    }

    std::vector<u32> indices;
    u32 restartIndex = 0xFF'FF'FF'FF;

    for(int i = 0; i < numOfVerticesX - 1; ++i)
    {
        for(int j = 0; j < numOfVerticesZ; ++j)
        {
            indices.push_back(j + numOfVerticesZ * (i + 1));
            indices.push_back(j + numOfVerticesZ * i);
        }

        indices.push_back(restartIndex);
    }

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(restartIndex);

    t.mesh = CreateMesh(&vertices[0], vertices.size(), terrainVertexAttribs[0].stride, &indices[0],
                        indices.size(), terrainVertexAttribs, ArrayCount(terrainVertexAttribs));
    t.mesh.drawMode = GL_TRIANGLE_STRIP;

    return t;
}

Terrain CreateTessellatedTerrainMesh(float *heightmap, glm::vec2 fullMapSize, float mapPortion,
                                     float mapScale, int meshStep, float yShift)
{
    Terrain t = {};

    int textureFlags = TextureFlag_Heightmap | TextureFlag_Filter_Min_Linear |
                       TextureFlag_Filter_Mag_Linear | TextureFlag_ClampToEdge;
    t.heightmapTexture = CreateGLTexture(heightmap, (int)fullMapSize.x, (int)fullMapSize.y, textureFlags);

    t.mapSize = fullMapSize * mapPortion;
    t.yShift = yShift;
    t.heightmap = heightmap;

    t.mapScale = mapScale;
    t.worldSize = (t.mapSize - 1.0f) * t.mapScale;

    glm::vec2 center = t.worldSize / 2.0f;

    int numOfVerticesX = 0;
    int numOfVerticesZ = 0;

    std::vector<TerrainVertex> vertices;
    for(int y = 0; y < 32; y += 1)
    {
        numOfVerticesX++;
        numOfVerticesZ = 0;

        float posX = ((y - 16.0f)) * 3.0f;

        for(int x = 0; x < 32; x += 1)
        {
            TerrainVertex vertex = {};

            float posZ = ((x - 16.0f)) * 3.0f;
            vertex.position = glm::vec3(posX, 0.0f, posZ);

            vertex.uv = glm::vec2(x, y) / glm::vec2(32.0f, 32.0f);

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

float GetTerrainHeight(Terrain *terrain, float x, float z)
{
    glm::vec2 worldPos = glm::vec2(x, z) + (terrain->worldSize / 2.0f);

    glm::vec2 mapPos = (worldPos / terrain->worldSize) * terrain->mapSize;
    mapPos = glm::clamp(mapPos, glm::vec2(0.0f), terrain->mapSize - 1.001f);

    glm::ivec2 mapPos0 = glm::ivec2(mapPos);
    glm::vec2 weights = glm::fract(mapPos);

    float h00 = terrain->heightmap[mapPos0.t + (int)terrain->mapSize.x * mapPos0.s];
    float h10 = terrain->heightmap[mapPos0.t + (int)terrain->mapSize.x * (mapPos0.s + 1)];
    float h01 = terrain->heightmap[(mapPos0.t + 1) + (int)terrain->mapSize.x * mapPos0.s];
    float h11 = terrain->heightmap[(mapPos0.t + 1) + (int)terrain->mapSize.x * (mapPos0.s + 1)];

    return glm::mix(glm::mix(h00, h10, weights.x), glm::mix(h01, h11, weights.x), weights.y);
}

//Approximate intersection of a ray with the terrain mesh which is cheaper
//than finding the intersection of each triangle of the terrain mesh with the ray
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
        t += terrain->mapScale;
    }

    return result;
}

void RenderTerrain(Game *game)
{
    Terrain *terrain = &game->terrain;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(terrain->shader);

    ShaderSetMatrix4(terrain->shader, "u_view", game->view);

    SetTexture(terrain->heightmapTexture.id, 0);
    ShaderSetInt(terrain->shader, "u_heightmap", 0);

    SetTexture(terrain->colorTexture.id, 1);
    ShaderSetInt(terrain->shader, "u_color", 1);

    SetTexture(game->perlinNoise2.id, 2);
    ShaderSetInt(terrain->shader, "u_noiseMap", 2);

    SetTexture(game->shadowMapFbo.depth.id, 3);
    ShaderSetInt(terrain->shader, "u_shadowMap", 3);

    ShaderSetFloat(terrain->shader, "u_texCoordsMultiplier", game->terrainUVMultiplier);
    ShaderSetFloat(terrain->shader, "u_mapScale", terrain->mapScale);

    glBindVertexArray(terrain->mesh.vao);

    if(terrain->mesh.drawMode == GL_PATCHES)
    {
        glPatchParameteri(GL_PATCH_VERTICES, 4);
    }

    glDrawElements(terrain->mesh.drawMode, terrain->mesh.indicesCount, GL_UNSIGNED_INT, 0);

    glDisable(GL_CULL_FACE);
}