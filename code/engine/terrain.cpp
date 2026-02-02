#include "terrain.h"

#include "defines.h"
#include "math_utils.h"

#include "game.h"
#include "shader.h"

#include <GL/glew.h>

#include <glm/gtx/intersect.hpp>

#include <stb_image.h>

//Loads a 16-bit PNG heightmap and generates a terrain mesh
Terrain CreateTerrain(char *heightmapPath, float maxHeight, float mapPortion, float mapScale, int meshStep, float yShift)
{
    Terrain t = {};

    int fullMapWidth, fullMapHeight, channels;
    stbi_info(heightmapPath, &fullMapWidth, &fullMapHeight, &channels);

    uint8 *image = 0;
    uint16 *image16 = 0;

    if(channels == 1)
    {
        image16 = stbi_load_16(heightmapPath, &fullMapWidth, &fullMapHeight, &channels, 0);
        t.yScale = (maxHeight / 65535.0f);
    }
    else
    {
        image = stbi_load(heightmapPath, &fullMapWidth, &fullMapHeight, &channels, 0);
        t.yScale = (maxHeight / 256.0f);
    }

    t.mapWidth = (int)(fullMapWidth * mapPortion);
    t.mapHeight = (int)(fullMapHeight * mapPortion);

    t.yShift = yShift;
    t.heightmap = (float *)calloc(t.mapWidth * t.mapHeight, sizeof(float));
    for(int i = 0; i < t.mapHeight; ++i)
    {
        for(int j = 0; j < t.mapWidth; ++j)
        {
            int sampleIndex = j + i * fullMapWidth;
            int destIndex = j + i * t.mapWidth;

            uint16 sample = (channels == 1) ? image16[sampleIndex] : image[sampleIndex * channels];
            t.heightmap[destIndex] = sample * t.yScale - t.yShift;
        }
    }
    stbi_image_free(image);

    std::vector<TerrainVertex> vertices;

    int numOfVerticesX = 0;
    int numOfVerticesZ = 0;

    t.mapScale = mapScale;
    t.worldWidth = (int)((t.mapHeight - 1) * t.mapScale);
    t.worldHeight = (int)((t.mapWidth - 1) * t.mapScale);

    glm::vec2 center = glm::vec2(t.worldWidth, t.worldHeight) / 2.0f;

    for(int i = 0; i < t.mapHeight; i += meshStep)
    {
        numOfVerticesX++;
        numOfVerticesZ = 0;
        for(int j = 0; j < t.mapWidth; j += meshStep)
        {
            TerrainVertex vertex = {};
            vertex.position.x = ((float)i * t.mapScale) - center.x;
            vertex.position.y = (float)t.heightmap[(int)(j + t.mapWidth * i)];
            vertex.position.z = ((float)j * t.mapScale) - center.y;
            vertex.uv.x = (float)j / (t.mapWidth - 1);
            vertex.uv.y = (float)i / (t.mapHeight - 1);

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
    x += terrain->worldWidth / 2.0f;
    z += terrain->worldHeight / 2.0f;

    float w = (float)terrain->mapWidth;
    float h = (float)terrain->mapHeight;
    float mapX = (x / terrain->worldWidth) * h;
    float mapZ = (z / terrain->worldHeight) * w;

    mapX = Clamp(0.0f, mapX, (float)terrain->mapHeight - 1.001f);
    mapZ = Clamp(0.0f, mapZ, (float)terrain->mapWidth - 1.001f);

    int mapX0 = (int)mapX;
    int mapZ0 = (int)mapZ;
    float a = mapX - mapX0;
    float b = mapZ - mapZ0;

    float h00 = terrain->heightmap[mapZ0 + terrain->mapWidth * mapX0];
    float h10 = terrain->heightmap[mapZ0 + terrain->mapWidth * (mapX0 + 1)];
    float h01 = terrain->heightmap[(mapZ0 + 1) + terrain->mapWidth * mapX0];
    float h11 = terrain->heightmap[(mapZ0 + 1) + terrain->mapWidth * (mapX0 + 1)];

    return ((1 - a) * (1 - b) * h00) + (a * (1 - b) * h10) + ((1 - a) * b * h01) + (a * b * h11);
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

glm::vec3 GetRayTerrainIntersection(Terrain *terrain, glm::vec3 rayOrigin, glm::vec3 rayDirection, float maxDist)
{
    glm::vec3 result = glm::vec3(0.0f);

    if(rayOrigin.y < GetTerrainHeight(terrain, rayOrigin.x, rayOrigin.z))
    {
        return result;
    }

    float t = 0.0;
    float prevT = 0.0f;

    while(t < maxDist)
    {
        glm::vec3 p = rayOrigin + rayDirection * t;

        float h = GetTerrainHeight(terrain, p.x, p.z);
        if(p.y < h)
        {
            //if the next point is below the heightmap, we passed the intersection point
            //and have to search between prevT and t to find the point which is
            //approximately close to the exact spot of the intersection
            return FindApproximateIntersectionPoint(terrain, rayOrigin, rayDirection, prevT, t, 4);
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

    SetTexture(game->terrain.texture.id, 0);
    ShaderSetInt(game->terrainShader, "u_terrainMap", 0);

    glBindVertexArray(game->terrain.mesh.vao);
    glDrawElements(GL_TRIANGLE_STRIP, game->terrain.mesh.indicesCount, GL_UNSIGNED_INT, 0);
}