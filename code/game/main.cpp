#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX

#ifdef PROFILER
#include "public/TracyClient.cpp"
#endif

#include <SDL3/SDL_main.h>

#include <json.hpp>
#include <glm/gtx/intersect.hpp>

#include "compile_list.h"

struct GrassDistributionResult
{
    glm::vec3 *positions;
    glm::ivec2 count;
};

GrassDistributionResult GenerateGrassPositions(Terrain *terrain, float density)
{
    GrassDistributionResult result = {};

    glm::vec2 cellSize = density * terrain->worldSize;
    glm::ivec2 cellCount = terrain->worldSize / cellSize;

    result.positions = (glm::vec3 *)calloc(cellCount.x * cellCount.y, sizeof(glm::vec3));
    result.count = cellCount;

    glm::vec2 center = glm::vec2(cellCount) / 2.0f;
    for(int y = 0; y < cellCount.y; y++)
    {

        for(int x = 0; x < cellCount.x; x++)
        {
            glm::vec2 gridPos = (glm::vec2(x, y) - center) * cellSize;
            gridPos += glm::vec2(RandomBetween(0.0f, cellSize.x), RandomBetween(0.0f, cellSize.y));

            glm::vec3 pos = glm::vec3(gridPos.x, GetTerrainHeight(terrain, gridPos.x, gridPos.y), gridPos.y);
            result.positions[x + cellCount.x * y] = pos;
        }
    }

    return result;
}

struct GrassData
{
    float angle;
    glm::vec3 offset;
};

int main(int argc, char *argv[])
{
    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    LoadTestScene(game);

    GrassDistributionResult result = GenerateGrassPositions(&game->terrain, 0.003f);
    game->grassCount = result.count.x * result.count.y * 2;

    GrassData *grassData = (GrassData *)calloc(game->grassCount, sizeof(GrassData));
    for(int grassId = 0; grassId < game->grassCount; grassId += 2)
    {
        float randomAngle = RandomBetween(0.0f, 360.0f);

        grassData[grassId].offset = result.positions[grassId / 2];
        grassData[grassId].angle = glm::radians(randomAngle);

        grassData[grassId + 1].offset = result.positions[grassId / 2];
        grassData[grassId + 1].angle = glm::radians(randomAngle + 90.0f);
    }

    game->grassQuad = CreateUnitQuadStripes();
    glBindVertexArray(game->grassQuad.vao);

    glGenBuffers(1, &game->grassInstancesVbo);
    glBindBuffer(GL_ARRAY_BUFFER, game->grassInstancesVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GrassData) * game->grassCount, grassData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GrassData), (void *)offsetof(GrassData, angle));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(GrassData), (void *)offsetof(GrassData, offset));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    game->lastFrame = SDL_GetPerformanceCounter();
    while(game->isRunning)
    {
        ProcessInput(&game->input);

        UpdateEditor(game);
        UpdateGame(game);

        RenderGame(game);

        if(IsFirstPress(&game->input, SDL_SCANCODE_U))
        {
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);

            glm::ivec2 imgSize = game->outlineFbo.color.size;
            std::vector<float> depthData(imgSize.x * imgSize.y);

            glReadPixels(0, 0, imgSize.x, imgSize.y, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());

            std::vector<u8> pixelData(imgSize.x * imgSize.y);
            for(int i = 0; i < imgSize.x * imgSize.y; ++i)
            {
                pixelData[i] = (u8)(depthData[i] * 255.0f);
            }

            stbi_flip_vertically_on_write(1);
            stbi_write_png("depth_output.png", imgSize.x, imgSize.y, 1, pixelData.data(), imgSize.x);
        }

        Uint64 thisFrame = SDL_GetPerformanceCounter();
        game->deltaTime = (thisFrame - game->lastFrame) / (float)game->perfFreq;
        game->lastFrame = thisFrame;
    }

    return 0;
}

