#include "terrain.h"

#include "defines.h"
#include "math_utils.h"

#include "game.h"
#include "shader.h"
#include "ray.h"

#include <GL/glew.h>

#include <glm/gtx/intersect.hpp>

#include <stb_image.h>

//Loads a 16-bit PNG heightmap and generates a terrain mesh
Terrain CreateTerrain(char *heightmapPath, float maxHeight, float mapPortion, float mapScale, int meshStep, float yShift)
{
    Terrain t = {};

    int x, y, channels;
    stbi_info(heightmapPath, &x, &y, &channels);


    uint8 *image = 0;
    uint16 *image16 = 0;

    if(channels == 1)
    {
        image16 = stbi_load_16(heightmapPath, &x, &y, &channels, 0);
        t.yScale = (maxHeight / 65535.0f);
    }
    else
    {
        image = stbi_load(heightmapPath, &x, &y, &channels, 0);
        t.yScale = (maxHeight / 256.0f);
    }

    glm::vec2 fullMapSize = glm::vec2(x, y);
    t.mapSize = fullMapSize * mapPortion;

    t.yShift = yShift;
    t.heightmap = (float *)calloc((int)(t.mapSize.x * t.mapSize.y), sizeof(float));
    for(int i = 0; i < t.mapSize.y; ++i)
    {
        for(int j = 0; j < t.mapSize.x; ++j)
        {
            int sampleIndex = j + i * (int)fullMapSize.x;
            int destIndex = j + i * (int)t.mapSize.x;

            uint16 sample = (channels == 1) ? image16[sampleIndex] : image[sampleIndex * channels];
            t.heightmap[destIndex] = sample * t.yScale - t.yShift;
        }
    }
    stbi_image_free(image);

    std::vector<TerrainVertex> vertices;

    int numOfVerticesX = 0;
    int numOfVerticesZ = 0;

    t.mapScale = mapScale;
    t.worldSize = (t.mapSize - 1.0f) * t.mapScale;

    glm::vec2 center = t.worldSize / 2.0f;

    for(int i = 0; i < t.mapSize.y; i += meshStep)
    {
        numOfVerticesX++;
        numOfVerticesZ = 0;

        float posX = ((float)i * t.mapScale) - center.x;

        for(int j = 0; j < t.mapSize.x; j += meshStep)
        {
            TerrainVertex vertex = {};
            vertex.position = glm::vec3(posX, t.heightmap[(int)(j + t.mapSize.x * i)], (j * t.mapScale) - center.y);
            vertex.uv = glm::vec2(j, i) / (t.mapSize - 1.0f);

            vertices.push_back(vertex);
            numOfVerticesZ++;
        }
    }

    uint32 restartIndex = 0xFF'FF'FF'FF;
    std::vector<uint32> indices;
    for(int i = 0; i < numOfVerticesX - 1; ++i)
    {
        for(int j = 0; j < numOfVerticesZ; ++j)
        {
            indices.push_back(j + numOfVerticesZ * i);
            indices.push_back(j + numOfVerticesZ * (i + 1));
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
    glUseProgram(game->terrainShader);
    ShaderSetMatrix4(game->terrainShader, "u_view", game->view);

    SetTexture(game->terrain.splatMap.id, 0);
    ShaderSetInt(game->terrainShader, "u_splatMap", 0);

    SetTexture(game->terrain.texture0.id, 1);
    ShaderSetInt(game->terrainShader, "u_texture0", 1);
    SetTexture(game->terrain.texture1.id, 2);
    ShaderSetInt(game->terrainShader, "u_texture1", 2);
    SetTexture(game->terrain.texture2.id, 3);
    ShaderSetInt(game->terrainShader, "u_texture2", 3);
    SetTexture(game->terrain.texture3.id, 4);
    ShaderSetInt(game->terrainShader, "u_texture3", 4);

    glBindVertexArray(game->terrain.mesh.vao);
    glDrawElements(GL_TRIANGLE_STRIP, game->terrain.mesh.indicesCount, GL_UNSIGNED_INT, 0);
}