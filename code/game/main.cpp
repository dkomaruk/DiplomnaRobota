#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX

#ifdef PROFILER
#include "public/TracyClient.cpp"
#endif

#include <SDL3/SDL_main.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#undef near
#undef far

#include <json.hpp>
#include <glm/gtx/intersect.hpp>

#include "compile_list.h"

int main(int argc, char *argv[])
{
    Game *game = GetGame();
    if(!InitGame(game))
    {
        return -1;
    }

    LoadTestScene(game);

    game->lastFrame = SDL_GetPerformanceCounter();
    while(game->isRunning)
    {
        ProcessInput(&game->input);

        UpdateFPSCamera(game);

        UpdateEditorUI(game);
        UpdateGame(game);

        RenderGame(game);

        if(IsFirstPress(&game->input, SDL_SCANCODE_U))
        {
            glBindFramebuffer(GL_FRAMEBUFFER, game->outlineFbo.id);

            glm::ivec2 imgSize = game->outlineFbo.color.size;
            std::vector<float> depthData(imgSize.x * imgSize.y);

            glReadPixels(0, 0, imgSize.x, imgSize.y, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());

            std::vector<uint8> pixelData(imgSize.x * imgSize.y);
            for(int i = 0; i < imgSize.x * imgSize.y; ++i)
            {
                pixelData[i] = (uint8)(depthData[i] * 255.0f);
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

