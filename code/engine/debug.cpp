#include "debug.h"

#include "defines.h"
#include "game.h"
#include "framebuffer.h"

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

void SaveFramebufferContents(Framebuffer fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);

    glm::ivec2 imgSize = fbo.color.size;
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