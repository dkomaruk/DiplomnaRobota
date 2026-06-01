#include "terrain_editor.h"

#include "game.h"
#include "noise.h"
#include "image.h"

#include <glm/vec2.hpp>

#include <stb_image.h>
#include <stb_image_write.h>

#include <GL/glew.h>

void SaveHeightmapCallback(void *userdata, const char * const *filelist, int filter)
{
    if(filelist && *filelist)
    {
        Game *game = (Game *)userdata;

        FILE *file = fopen(filelist[0], "wb");
        if(file)
        {
            int width = (int)game->terrain.mapSize.x;
            int height = (int)game->terrain.mapSize.y;

            fwrite(&width, sizeof(int), 1, file);
            fwrite(&height, sizeof(int), 1, file);
            fwrite(game->terrain.heightmap, sizeof(float), width * height, file);

            fclose(file);
        }
    }
}

void LoadHeightmapCallback(void *userdata, const char * const *filelist, int filter)
{
    if(filelist && *filelist)
    {
        Game *game = (Game *)userdata;

        game->editor.regenerateTerrain = true;
        game->editor.heightmapPath = std::string(filelist[0]);
    }
}

void GenerateNewTerrain(Game *game, float *heightmap, glm::vec2 size, int patchSize, float mapScale)
{
    DeleteMesh(&game->terrain.mesh);
    glDeleteTextures(1, &game->terrain.heightmapTexture.id);
    free(game->terrain.heightmap);

    Terrain terrain = CreateTessellatedTerrainMesh(heightmap, size, patchSize, mapScale, 0.0f);
    terrain.shader = GetShader(game, "tessellated_terrain");
    terrain.minTessDist = game->terrain.minTessDist;
    terrain.maxTessDist = game->terrain.maxTessDist;
    terrain.minTessLevel = game->terrain.minTessLevel;
    terrain.maxTessLevel = game->terrain.maxTessLevel;
    terrain.colorTexture = game->terrain.colorTexture;

    game->terrain = terrain;
}

void UpdateTerrainEditorUI(Game *game, bool *windowState, ImGuiWindowFlags flags)
{
    Editor *editor = &game->editor;
    Terrain *terrain = &game->terrain;

    ImGui::Begin("Terrain Editor", windowState, flags | ImGuiWindowFlags_HorizontalScrollbar);

    static glm::ivec2 gridSize = glm::ivec2(3);
    static int octaves = 3;
    static float persistence = 0.5f;
    static float lacunarity = 2.0f;
    static float maxHeight = 20.0f;
    static glm::ivec2 size = glm::ivec2(1024, 1024);
    static float mapScale = 0.1f;
    static float yOffset = 0.0f;
    static int patchSize = 16;

    if(ImGui::CollapsingHeader("Terrain Generation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Image(game->perlinNoise.id, ImVec2(256.0f, 256.0f));

        ImGui::InputInt2("Initial Grid Size", &gridSize[0]);
        ImGui::InputInt("Octaves", &octaves);
        ImGui::InputFloat("Persistence", &persistence, 0.05f);
        ImGui::InputFloat("Lacunarity", &lacunarity, 0.05f);
        ImGui::InputFloat("Max Height", &maxHeight, 0.05f);
        ImGui::InputInt2("Texture Size", &size[0]);
        ImGui::InputFloat("Map Scale", &mapScale, 0.05f);
        ImGui::InputFloat("Y Offset", &yOffset, 0.05f);
        ImGui::InputFloat("UV Multiplier", &game->terrainUVMultiplier, 0.05f);
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    if(ImGui::CollapsingHeader("Terrain Tessellation Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputInt("Patch Size", &patchSize);
        patchSize = SDL_clamp(patchSize, 4, 1024);

        ImGui::DragFloat("Min tessellation distance", &terrain->minTessDist);
        ImGui::DragFloat("Max tessellation distance", &terrain->maxTessDist);
        ImGui::DragFloat("Min tessellation level", &terrain->minTessLevel, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Max tessellation level", &terrain->maxTessLevel, 0.1f);
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    TerrainBrush *brush = &editor->terrainBrush;
    if(ImGui::CollapsingHeader("Terrain Sculpting", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Is Enabled", &editor->terrainSculpting);
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
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    if(ImGui::Button("Generate"))
    {
        float *perlinNoise = GeneratePerlinNoise2(glm::vec2(size), gridSize, octaves, persistence, lacunarity);

        u8 *perlinNoiseImage = NoiseToImage(perlinNoise, size);
        glDeleteTextures(1, &game->perlinNoise.id);
        game->perlinNoise = CreateGLTexture(perlinNoiseImage, size.x, size.y);

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

        GenerateNewTerrain(game, heightmap, size, patchSize, mapScale);

        free(perlinNoise);
        free(perlinNoiseImage);
    }

    static SDL_DialogFileFilter filters[] = {{"Heightmap Data", "bin"}};

    ImGui::SameLine();
    if(ImGui::Button("Save"))
    {
        SDL_ShowSaveFileDialog(SaveHeightmapCallback, (void *)game, game->window, filters, 1, 0);
    }
    ImGui::SameLine();
    if(ImGui::Button("Load"))
    {
        SDL_ShowOpenFileDialog(LoadHeightmapCallback, (void *)game, game->window, filters, 1, 0, false);
    }
    if(editor->regenerateTerrain)
    {
        FILE *file = fopen(editor->heightmapPath.c_str(), "rb");
        if(file)
        {
            int x = 0, y = 0;
            fread(&x, sizeof(int), 1, file);
            fread(&y, sizeof(int), 1, file);

            if(x > 0 && y > 0)
            {
                float *heightmapData = (float *)malloc(x * y * sizeof(float));
                if(heightmapData)
                {
                    fread(heightmapData, sizeof(float), x * y, file);
                    GenerateNewTerrain(game, heightmapData, glm::vec2(x, y), 16, 0.1f);
                }
            }

            fclose(file);
        }

        editor->regenerateTerrain = false;
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
    Input *input = &game->input;

    UpdateTerrainEditorUI(game, &editor->terrainGeneratorWindow, flags);

    if(IsFirstPress(input, SDL_SCANCODE_F1)) editor->terrainBrush.type = TerrainBrush_Add;
    if(IsFirstPress(input, SDL_SCANCODE_F2)) editor->terrainBrush.type = TerrainBrush_Flatten;
    if(IsFirstPress(input, SDL_SCANCODE_F3)) editor->terrainBrush.type = TerrainBrush_Smooth;
    if(IsFirstPress(input, SDL_SCANCODE_F4)) editor->terrainBrush.type = TerrainBrush_Noise;

    if(editor->terrainSculpting)
    {
        TerrainBrush *brush = &editor->terrainBrush;

        brush->strength = (IsKeyDown(input, SDL_SCANCODE_LCTRL) ? -1.0f : 1.0f) * abs(brush->strength);

        //Hotkeys to change the radius/strength of the sculpting brush
        if(IsKeyDown(input, SDL_SCANCODE_F))
        {
            if(IsKeyJustDown(input, SDL_SCANCODE_F))
            {
                editor->lastMousePos = input->mousePos;
                SDL_HideCursor();
                SDL_SetWindowRelativeMouseMode(game->window, true);
            }
            else
            {
                SDL_WarpMouseInWindow(game->window, editor->lastMousePos.x, editor->lastMousePos.y);
            }

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
        else
        {
            if(IsKeyJustUp(input, SDL_SCANCODE_F))
            {
                SDL_WarpMouseInWindow(game->window, editor->lastMousePos.x, editor->lastMousePos.y);
                input->mousePos = editor->lastMousePos;
                SDL_ShowCursor();
                SDL_SetWindowRelativeMouseMode(game->window, false);
            }

            glm::vec2 mousePos = input->isCursorHidden ? RECT_HALF(game->windowSize) : input->mousePos;
            mousePos.y = (int)game->windowSize.y - mousePos.y;

            Ray pickingRay = CastPickingRay(game, mousePos);
            float visibleRayLength = 2000.0f;

            glm::vec3 intersectionPoint = GetRayTerrainIntersection(&game->terrain, &pickingRay, visibleRayLength);
            editor->terrainBrush.center = intersectionPoint;

            bool isMouseInPosition = IsButtonDown(input, MOUSE_LEFT) && !input->isMouseCapturedByImgui;
            bool shouldSculpt = (editor->terrainBrush.type != TerrainBrush_Noise) || IsFirstClick(input, MOUSE_LEFT);

            if(isMouseInPosition && shouldSculpt)
            {
                SculptTerrain(&game->terrain, intersectionPoint, &editor->terrainBrush, game->deltaTime);
            }
        }

    }

}