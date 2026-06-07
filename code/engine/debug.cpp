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

const glm::vec4 shadowVolumeCornersNDC[8] = {
    glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
    glm::vec4( 1.0f, -1.0f, -1.0f, 1.0f),
    glm::vec4( 1.0f,  1.0f, -1.0f, 1.0f),
    glm::vec4(-1.0f,  1.0f, -1.0f, 1.0f),
    glm::vec4(-1.0f, -1.0f,  1.0f, 1.0f),
    glm::vec4( 1.0f, -1.0f,  1.0f, 1.0f),
    glm::vec4( 1.0f,  1.0f,  1.0f, 1.0f),
    glm::vec4(-1.0f,  1.0f,  1.0f, 1.0f)
};

void CreateShadowVolumeLines(Line *lines, GLuint shader)
{
    for(int i = 0; i < 12; i++)
    {
        lines[i] = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), shader, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void UpdateShadowVolumeLines(Line *lines, glm::mat4 lightViewProj)
{
    glm::mat4 invLightViewProj = glm::inverse(lightViewProj);
    glm::vec3 worldCorners[8];

    for(int i = 0; i < 8; i++)
    {
        glm::vec4 worldPos = invLightViewProj * shadowVolumeCornersNDC[i];
        worldCorners[i] = glm::vec3(worldPos) / worldPos.w;
    }

    int edgeIndices[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for(int i = 0; i < 12; i++)
    {
        glm::vec3 startPoint = worldCorners[edgeIndices[i][0]];
        glm::vec3 endPoint = worldCorners[edgeIndices[i][1]];
        UpdateLine(&lines[i], startPoint, endPoint);
    }
}