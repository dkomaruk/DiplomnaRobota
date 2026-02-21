#include "noise.h"

#include "math_utils.h"

#include <SDL3/SDL.h>
#include <glm/gtx/quaternion.hpp>

#include <stdlib.h>

uint8 *GenerateValueNoise(glm::vec2 size)
{
    int numOfChannels = 4;
    uint8 *noise = (uint8 *)calloc((int)(size.x * size.y) * numOfChannels, sizeof(uint8));

    glm::ivec2 gridSize = glm::ivec2(32);

    uint8 *values = (uint8 *)calloc(gridSize.x * gridSize.y, sizeof(uint8));
    for(int y = 0; y < gridSize.y; y++)
    {
        for(int x = 0; x < gridSize.x; x++)
        {
            values[x + gridSize.x * y] = (uint8)(((float)rand() / RAND_MAX) * 255.0f);
        }
    }

    for(int y = 0; y < size.y; y++)
    {
        for(int x = 0; x < size.x; x++)
        {
            glm::vec2 pos = (glm::vec2(x, y) / size) * glm::vec2(gridSize);
            pos = glm::clamp(pos, glm::vec2(0.0f), glm::vec2(gridSize));

            glm::ivec2 pos0 = glm::ivec2(pos);
            glm::vec2 weights = glm::fract(pos);

            uint8 v00 = values[pos0.x + gridSize.x * pos0.y];
            uint8 v01 = values[pos0.x + gridSize.x * (pos0.y + 1)];
            uint8 v10 = values[(pos0.x + 1) + gridSize.x * pos0.y];
            uint8 v11 = values[(pos0.x + 1) + gridSize.x * (pos0.y + 1)];

            weights = glm::smoothstep(0.0f, 1.0f, weights);
            uint8 value = glm::mix(glm::mix(v00, v10, weights.x), glm::mix(v01, v11, weights.x), weights.y);

            int id = (x + (int)size.x * y) * numOfChannels;
            noise[id + 0] = value;
            noise[id + 1] = value;
            noise[id + 2] = value;
            noise[id + 3] = 255;
        }
    }

    free(values);

    return noise;
}

uint8 *GeneratePerlinNoise(glm::vec2 size, uint8 temp)
{
    int numOfChannels = 4;
    uint8 *noise = (uint8 *)calloc((int)(size.x * size.y) * numOfChannels, sizeof(uint8));

    glm::ivec2 gridSize = glm::ivec2(32);
    glm::vec2 *gradients = (glm::vec2 *)calloc(gridSize.x * gridSize.y, sizeof(glm::vec2));

    for(int y = 0; y < gridSize.y; y++)
    {
        for(int x = 0; x < gridSize.x; x++)
        {
            gradients[x + gridSize.x * y] = glm::normalize(glm::vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX) * 2.0f - 1.0f);
        }
    }

    for(int y = 0; y < size.y; y++)
    {
        for(int x = 0; x < size.x; x++)
        {
            glm::vec2 pos = (glm::vec2(x, y) / size) * glm::vec2(gridSize);

            glm::ivec2 pos0 = glm::ivec2(pos);
            glm::vec2 weights = glm::fract(pos);

            int x0 = pos0.x % gridSize.x;
            int x1 = (pos0.x + 1) % gridSize.x;
            int y0 = pos0.y % gridSize.y;
            int y1 = (pos0.y + 1) % gridSize.y;

            float v00 = glm::dot((weights - glm::vec2(0.0f, 0.0f)), gradients[x0 + gridSize.x * y0]);
            float v10 = glm::dot((weights - glm::vec2(1.0f, 0.0f)), gradients[x1 + gridSize.x * y0]);
            float v01 = glm::dot((weights - glm::vec2(0.0f, 1.0f)), gradients[x0 + gridSize.x * y1]);
            float v11 = glm::dot((weights - glm::vec2(1.0f, 1.0f)), gradients[x1 + gridSize.x * y1]);

            //weights = glm::smoothstep(0.0f, 1.0f, weights); //Causes discontinuity at grid cell borders
            weights = weights * weights * weights * (weights * (weights * 6.0f - 15.0f) + 10.0f); //6x^5 - 15x^4 + 10x^3 = x^3 * (x * (6x - 15) + 10)
            float blendedValue = glm::mix(glm::mix(v00, v10, weights.x), glm::mix(v01, v11, weights.x), weights.y);

            //The largest distance vector from pos to grid cell corner is (1, 1) which has length sqrt(2)= +-1.414
            //Gradient vector is normalized and has length of 1
            //Thus the dot product between the two is in range +-1.414
            //The blend between the four dot products can at most be +-0.707 right at the center of the grid cell (0.5, 0.5)
            float normalizedValue = glm::clamp((((blendedValue / 0.707f) + 1.0f) / 2.0f), 0.0f, 1.0f);
            uint8 value = (uint8)(normalizedValue * 255.0f);

            int id = (x + (int)size.x * y) * numOfChannels;
            noise[id + 0] = value;
            noise[id + 1] = value;
            noise[id + 2] = value;
            noise[id + 3] = 255;
        }
    }

    free(gradients);

    return noise;
}
