#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX

//TODO: Replace with SDL_ShowOpenFileDialog
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#ifdef PROFILER
#include "public/TracyClient.cpp"
#endif

#undef near //There are defined in windows headers and collide with some variable names
#undef far

//Has to be included at the start because of compilation errors otherwise
//TODO: Replace with a less heavy library
#include <json.hpp>

//These GLM extensions have to be included before stb_vorbis.c for some reason. Otherwise it doesn't compile
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

    //TODO: Terrain tesselation
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

        if(IsFirstPress(&game->input, SDL_SCANCODE_U))
        {
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);
            int width = 1024, height = 1024;
            std::vector<float> depthData(width * height);

            glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());

            std::vector<unsigned char> pixelData(width * height);

            for (int i = 0; i < width * height; ++i) {
                pixelData[i] = static_cast<unsigned char>(depthData[i] * 255.0f);
            }

            stbi_flip_vertically_on_write(1);
            stbi_write_png("depth_output.png", width, height, 1, pixelData.data(), width);
        }


        Uint64 thisFrame = SDL_GetPerformanceCounter();
        game->deltaTime = (thisFrame - game->lastFrame) / (float)game->perfFreq;
        game->lastFrame = thisFrame;
    }

    return 0;
}

