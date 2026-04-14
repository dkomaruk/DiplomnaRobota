#include "debug.h"

#include "defines.h"
#include "game.h"

#include <SDL3/SDL.h>

u64 debugStart, debugEnd;
int debugCounter = 0;
float debugAccum = 0.0f;

inline void StartProfiling(bool reset)
{
    if(reset)
    {
        debugCounter = 0;
        debugAccum = 0.0f;
    }

    debugStart = SDL_GetPerformanceCounter();
}

inline void EndProfiling(const std::string& extraInfo)
{
    debugEnd = SDL_GetPerformanceCounter();

    static float debugPerfFreq = (float)GetGame()->perfFreq;
    float elapsed = (debugEnd - debugStart) / debugPerfFreq;
    debugCounter += 1;
    debugAccum += elapsed;
    SDL_Log("%d: %f | Total: %f (%s)", debugCounter, elapsed, debugAccum, extraInfo.c_str());

}