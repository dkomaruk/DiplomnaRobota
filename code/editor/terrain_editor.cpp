#include "terrain_editor.h"

#include "game.h"
#include "noise.h"

#include <glm/vec2.hpp>

void UpdateTerrainEditorUI(Game *game, bool *windowState, ImGuiWindowFlags flags)
{
    ImGui::Begin("Terrain Generator", windowState, flags | ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::Image(game->perlinNoise.id, ImVec2(256.0f, 256.0f));

    static glm::ivec2 gridSize = glm::ivec2(32);
    ImGui::InputInt2("Initial Grid Size", &gridSize[0]);

    static int octaves = 1;
    ImGui::InputInt("Octaves", &octaves);

    static float persistence = 0.5f;
    ImGui::InputFloat("Persistence", &persistence, 0.05f);

    static float lacunarity = 2.0f;
    ImGui::InputFloat("Lacunarity", &lacunarity, 0.05f);

    static float maxHeight = 20.0f;
    ImGui::InputFloat("Max Height", &maxHeight, 0.05f);

    static glm::ivec2 size = glm::ivec2(1024, 1024);
    ImGui::InputInt2("Size", &size[0]);

    static float mapScale = 0.1f;
    ImGui::InputFloat("Map Scale", &mapScale, 0.05f);

    static float yOffset = 0.0f;
    ImGui::InputFloat("Y Offset", &yOffset, 0.05f);

    ImGui::InputFloat("UV Multiplier", &game->terrainUVMultiplier, 0.05f);

    static int meshStep = 8;
    ImGui::InputInt("Mesh Step", &meshStep);

    meshStep = SDL_clamp(meshStep, 1, 100);

    if(ImGui::Button("Generate"))
    {
        glDeleteTextures(1, &game->perlinNoise.id);

        float *perlinNoise = GeneratePerlinNoise(glm::vec2(size), gridSize, octaves, persistence, lacunarity);

        u8 *perlinNoiseImage = NoiseToImage(perlinNoise, size);
        game->perlinNoise = CreateGLTexture(perlinNoiseImage, size.x, size.y);
        free(perlinNoiseImage);

        DeleteMesh(&game->terrain.mesh);
        free(game->terrain.heightmap);
        glDeleteTextures(1, &game->terrain.heightmapTexture.id);

        float *heightmap = (float *)calloc((int)(size.x * size.y), sizeof(float));
        for(int i = 0; i < size.y; ++i)
        {
            for(int j = 0; j < size.x; ++j)
            {
                int sampleIndex = j + i * (int)size.x;
                int destIndex = j + i * (int)size.x;

                heightmap[destIndex] = perlinNoise[sampleIndex] * maxHeight - yOffset;
            }
        }

        Terrain terrain = CreateTessellatedTerrainMesh(heightmap, size, 1.0f, mapScale, meshStep, 0.0f);
        terrain.shader = game->tessellatedTerrainShader;

        //Terrain terrain = CreateTerrainMesh(heightmap, size, 1.0f, mapScale, meshStep, 0.0f);
        //terrain.shader = game->terrainShader;

        terrain.colorTexture = game->terrain.colorTexture;

        //int textureFlags = TextureFlag_Heightmap | TextureFlag_Filter_Min_Linear |
        //                TextureFlag_Filter_Mag_Linear | TextureFlag_ClampToEdge;
        //terrain.heightmapTexture = CreateGLTexture(heightmap, size.x, size.y, textureFlags);

        game->terrain = terrain;

        free(perlinNoise);
    }
    ImGui::End();
}