#include "terrain_editor.h"

#include "game.h"
#include "noise.h"

#include <glm/vec2.hpp>

#include <GL/glew.h>

void UpdateTerrainEditorUI(Game *game, bool *windowState, ImGuiWindowFlags flags)
{
    Editor *editor = &game->editor;
    Terrain *terrain = &game->terrain;

    ImGui::Begin("Terrain Generator", windowState, flags | ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::Image(game->perlinNoise.id, ImVec2(256.0f, 256.0f));

    static glm::ivec2 gridSize = glm::ivec2(3);
    ImGui::InputInt2("Initial Grid Size", &gridSize[0]);

    static int octaves = 3;
    ImGui::InputInt("Octaves", &octaves);

    static float persistence = 0.5f;
    ImGui::InputFloat("Persistence", &persistence, 0.05f);

    static float lacunarity = 2.0f;
    ImGui::InputFloat("Lacunarity", &lacunarity, 0.05f);

    static float maxHeight = 20.0f;
    ImGui::InputFloat("Max Height", &maxHeight, 0.05f);

    static glm::ivec2 size = glm::ivec2(1024, 1024);
    ImGui::InputInt2("Texture Size", &size[0]);

    static float mapScale = 0.1f;
    ImGui::InputFloat("Map Scale", &mapScale, 0.05f);

    static float yOffset = 0.0f;
    ImGui::InputFloat("Y Offset", &yOffset, 0.05f);

    ImGui::InputFloat("UV Multiplier", &game->terrainUVMultiplier, 0.05f);

    static int patchSize = 16;
    ImGui::InputInt("Patch Size", &patchSize);
    patchSize = SDL_clamp(patchSize, 4, 100);

    ImGui::DragFloat("Min tessellation distance", &terrain->minTessDist);
    ImGui::DragFloat("Max tessellation distance", &terrain->maxTessDist);
    ImGui::DragFloat("Min tessellation level", &terrain->minTessLevel, 0.1f, 0.1f, 100.0f);
    ImGui::DragFloat("Max tessellation level", &terrain->maxTessLevel, 0.1f);

    TerrainBrush *brush = &editor->terrainBrush;
    ImGui::Checkbox("Terrain Sculpting", &editor->terrainSculpting);
    if(editor->terrainSculpting)
    {
        ImGui::InputFloat("Brush Strength", &brush->strength);
        ImGui::InputFloat("Brush Radius", &brush->radius);

        ImGui::Combo("Brush Type", &brush->type, "Add\0Flatten\0Smooth\0Noise\0\0");
        if(brush->type == TerrainBrush_Smooth)
        {
            const int kernels[] = {3, 5, 7, 9, 11, 13, 15};
            static int kernelId = 0;
            if(ImGui::Combo("Brush Kernel Size", &kernelId, " 3\0 5\0 7\0 9\0 11\0 13\0 15\0\0"))
                brush->kernelSize = kernels[kernelId];
        }
        else if(brush->type == TerrainBrush_Noise)
        {
            ImGui::Combo("Brush Noise Type", &brush->noiseType,
                         "Simplex2\0Simplex2S\0Cellular\0Perlin\0Value Cubic\0Value\0\0");

            ImGui::DragFloat("Noise Frequency", &brush->noiseFreq, 0.005f, 0.001f, 1.0f);
            ImGui::InputInt("Noise Octaves", &brush->octaves);
        }
    }

    if(ImGui::Button("Generate"))
    {
        glDeleteTextures(1, &game->perlinNoise.id);

        //float *perlinNoise = GeneratePerlinNoise(glm::vec2(size), gridSize, octaves, persistence, lacunarity);
        float *perlinNoise = GeneratePerlinNoise2(glm::vec2(size), gridSize, octaves, persistence, lacunarity);

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

        Terrain terrain = CreateTessellatedTerrainMesh(heightmap, size, patchSize, mapScale, 0.0f);
        terrain.shader = game->tessellatedTerrainShader;
        terrain.minTessDist = game->terrain.minTessDist;
        terrain.maxTessDist = game->terrain.maxTessDist;
        terrain.minTessLevel = game->terrain.minTessLevel;
        terrain.maxTessLevel = game->terrain.maxTessLevel;

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

void SculptTerrain(Terrain *terrain, glm::vec3 intersectionPoint, TerrainBrush *brush, float deltaTime)
{
    glm::vec2 worldPos = glm::vec2(intersectionPoint.x, intersectionPoint.z) + terrain->halfWorldSize;
    glm::vec2 mapCenter = (worldPos / terrain->worldSize) * terrain->mapSize;

    glm::ivec2 min = glm::max(glm::vec2(0.0f), glm::floor(mapCenter - brush->radius));
    glm::ivec2 max = glm::min(terrain->mapSize - 1.0f, glm::ceil(mapCenter + brush->radius));

    glm::ivec2 center = glm::clamp(mapCenter, glm::vec2(0.0f), terrain->mapSize - 1.0f);
    float centerHeight = terrain->heightmap[center.x + center.y * (int)terrain->mapSize.x];

    fnl_state brushNoise;
    if(brush->type == TerrainBrush_Noise)
    {
        brushNoise = fnlCreateState();
        brushNoise.noise_type = (fnl_noise_type)brush->noiseType;
        brushNoise.fractal_type = FNL_FRACTAL_FBM;
        brushNoise.octaves = brush->octaves;
        brushNoise.seed = rand();
    }

    for(int y = min.y; y <= max.y; ++y)
    {
        for(int x = min.x; x <= max.x; ++x)
        {
            float distance = glm::length(glm::vec2(x, y) - mapCenter);
            if(distance < brush->radius)
            {
                int index = x + y * (int)terrain->mapSize.x;
                float falloff = glm::smoothstep(brush->radius, 0.0f, distance);

                switch(brush->type)
                {
                    case TerrainBrush_Add:
                    {
                        terrain->heightmap[index] += brush->strength * falloff * deltaTime;
                    } break;

                    case TerrainBrush_Flatten:
                    {
                        float currentHeight = terrain->heightmap[index];
                        float weight = glm::clamp(abs(brush->strength) * falloff * deltaTime, 0.0f, 1.0f);
                        terrain->heightmap[index] = glm::mix(currentHeight, centerHeight, weight);
                    } break;

                    case TerrainBrush_Smooth:
                    {
                        float currentHeight = terrain->heightmap[index];
                        float averageHeight = 0.0f;
                        int sampleCount = 0;

                        int r = (brush->kernelSize - 1) / 2;
                        for(int dy = -r; dy <= r; ++dy)
                        {
                            for(int dx = -r; dx <= r; ++dx)
                            {
                                int nx = glm::clamp(x + dx, 0, (int)terrain->mapSize.x - 1);
                                int ny = glm::clamp(y + dy, 0, (int)terrain->mapSize.y - 1);
                                averageHeight += terrain->heightmap[nx + ny * (int)terrain->mapSize.x];
                                sampleCount++;
                            }
                        }

                        averageHeight /= (float)sampleCount;

                        float weight = glm::clamp(abs(brush->strength) * falloff * deltaTime, 0.0f, 1.0f);
                        terrain->heightmap[index] = glm::mix(currentHeight, averageHeight, weight);
                    } break;

                    case TerrainBrush_Noise:
                    {
                        float noiseVal = fnlGetNoise2D(&brushNoise, x * brush->noiseFreq, y * brush->noiseFreq);
                        terrain->heightmap[index] += noiseVal * brush->strength * falloff * deltaTime;
                    } break;
                }
            }
        }
    }

    //Update changed part of the heightmap texture on the GPU
    int width = max.x - min.x + 1;
    int height = max.y - min.y + 1;

    glBindTexture(GL_TEXTURE_2D, terrain->heightmapTexture.id);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, (int)terrain->mapSize.x);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, min.x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, min.y);

    glTexSubImage2D(GL_TEXTURE_2D, 0, min.x, min.y, width, height, GL_RED, GL_FLOAT, terrain->heightmap);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
}

void UpdateTerrainEditor(Game *game, ImGuiWindowFlags flags)
{
    Editor *editor = &game->editor;

    UpdateTerrainEditorUI(game, &editor->terrainGeneratorWindow, flags);

    if(editor->terrainSculpting)
    {
        Input *input = &game->input;
        TerrainBrush *brush = &editor->terrainBrush;

        brush->strength = (IsKeyDown(input, SDL_SCANCODE_LCTRL) ? -1.0f : 1.0f) * abs(brush->strength);

        //Hotkeys to change the radius/strength of the sculpting brush
        if(IsKeyDown(input, SDL_SCANCODE_F))
        {
            float dir = input->mouseDelta.x * game->deltaTime;
            if(IsKeyDown(input, SDL_SCANCODE_LSHIFT))
            {
                brush->strength += 5.0f * dir;
                brush->strength = glm::clamp(brush->strength, 0.0f, 1000.0f);
            }
            else
            {
                brush->radius += 10.0f * dir;
                brush->radius = glm::clamp(brush->radius, 0.0f, 1000.0f);
            }
        }

        if(IsButtonDown(input, MOUSE_LEFT) && !input->isMouseCapturedByImgui)
        {
            glm::vec2 mousePos = input->isCursorHidden ? RECT_HALF(game->windowSize) : input->mousePos;
            mousePos.y = (int)game->windowSize.y - mousePos.y;

            Ray pickingRay = CastPickingRay(game, mousePos);
            float visibleRayLength = 2000.0f;
            glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, &pickingRay, visibleRayLength);

            if((editor->terrainBrush.type != TerrainBrush_Noise) || IsFirstClick(input, MOUSE_LEFT))
            {
                SculptTerrain(&game->terrain, intersectionPoint, &editor->terrainBrush, game->deltaTime);
            }
        }
    }

}