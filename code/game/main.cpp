#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX

#ifdef PROFILER
#include "public/TracyClient.cpp"
#endif

#include <SDL3/SDL_main.h>

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

        UpdateEditor(game);
        UpdateGame(game);

        RenderGame(game);

        Uint64 thisFrame = SDL_GetPerformanceCounter();
        game->deltaTime = (thisFrame - game->lastFrame) / (float)game->perfFreq;
        game->lastFrame = thisFrame;
    }

    return 0;
}

