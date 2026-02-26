#define GLM_ENABLE_EXPERIMENTAL
#define IMGUI_DEFINE_MATH_OPERATORS

//TODO: Replace with SDL_ShowOpenFileDialog
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#undef near //There are defined in windows headers and collide with some variable names
#undef far

//Has to be included at the start because of compilation errors otherwise
//TODO: Replace with a less heavy library
#include <json.hpp>

//GLM extensions have to be included before any other glm header file for some reason, otherwise it doesn't compile
#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "compile_list.h"

#include <GL/glew.h>

#include <SDL3/SDL_main.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <time.h>

#include <imgui.h>

#include <expat.h>

int main(int argc, char *argv[])
{
    srand((uint32)time(0));

    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    LoadAssets(game);

    glm::vec2 size = glm::vec2(256.0f, 256.0f);
    uint8 *valueNoise = GenerateValueNoise(size);
    game->valueNoise = CreateGLTexture(valueNoise, (int)size.x, (int)size.y);
    free(valueNoise);

    float *perlinNoise = GeneratePerlinNoise(size, glm::ivec2(32), 4, 0.5f, 2.0f);

    uint8 *perlinNoiseImage = (uint8 *)calloc((int)(size.x * size.y) * 4, sizeof(uint8));
    for(int y = 0; y < size.y; y++)
    {
        for(int x = 0; x < size.x; x++)
        {
            uint8 value = (uint8)(glm::clamp(perlinNoise[x + (int)size.x * y], 0.0f, 1.0f) * 255.0f);

            int id = (x + (int)size.x * y) * 4;
            perlinNoiseImage[id + 0] = value;
            perlinNoiseImage[id + 1] = value;
            perlinNoiseImage[id + 2] = value;
            perlinNoiseImage[id + 3] = 255;
        }
    }

    game->perlinNoise = CreateGLTexture(perlinNoiseImage, (int)size.x, (int)size.y);
    free(perlinNoiseImage);

    //TODO: Terrain tesselation, terrain normal calculation, terrain lighting
    //TODO: Vegetation on terrain (trees, grass)
    //TODO: Shadows (shadow mapping)

    //Main game loop start
    game->lastFrame = SDL_GetPerformanceCounter();
    while(game->isRunning)
    {
        ProcessInput(&game->input);

        UpdateEditorUI(game);
        UpdateGame(game);

        RenderGame(game);

        Uint64 thisFrame = SDL_GetPerformanceCounter();
        game->deltaTime = (thisFrame - game->lastFrame) / (float)game->perfFreq;
        game->lastFrame = thisFrame;
    }

    return 0;
}

