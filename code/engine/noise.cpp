#include "noise.h"

#include "math_utils.h"

#include <SDL3/SDL.h>
#include <glm/gtx/quaternion.hpp>

#include <stdlib.h>

u8 *GenerateValueNoise(glm::vec2 size)
{
    int numOfChannels = 4;
    u8 *noise = (u8 *)calloc((int)(size.x * size.y) * numOfChannels, sizeof(u8));

    glm::ivec2 gridSize = glm::ivec2(32);

    u8 *values = (u8 *)calloc(gridSize.x * gridSize.y, sizeof(u8));
    for(int y = 0; y < gridSize.y; y++)
    {
        for(int x = 0; x < gridSize.x; x++)
        {
            values[x + gridSize.x * y] = (u8)(((float)rand() / RAND_MAX) * 255.0f);
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

            u8 v00 = values[pos0.x + gridSize.x * pos0.y];
            u8 v01 = values[pos0.x + gridSize.x * (pos0.y + 1)];
            u8 v10 = values[(pos0.x + 1) + gridSize.x * pos0.y];
            u8 v11 = values[(pos0.x + 1) + gridSize.x * (pos0.y + 1)];

            weights = glm::smoothstep(0.0f, 1.0f, weights);
            u8 value = glm::mix(glm::mix(v00, v10, weights.x), glm::mix(v01, v11, weights.x), weights.y);

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

u8 *NoiseToImage(float *noise, glm::vec2 size)
{
    u8 *image = (u8 *)calloc((int)(size.x * size.y) * 4, sizeof(u8));
    for(int y = 0; y < size.y; y++)
    {
        for(int x = 0; x < size.x; x++)
        {
            u8 value = (u8)(glm::clamp(noise[x + (int)size.x * y], 0.0f, 1.0f) * 255.0f);

            int id = (x + (int)size.x * y) * 4;
            image[id + 0] = value;
            image[id + 1] = value;
            image[id + 2] = value;
            image[id + 3] = 255;
        }
    }

    return image;
}

float SamplePerlin(glm::vec2 pos, glm::vec2* gradientTable, glm::ivec2 gradientTableSize)
{
    glm::ivec2 pos0 = glm::ivec2(glm::floor(pos));
    glm::vec2 weights = glm::fract(pos);

    int x0 = pos0.x % gradientTableSize.x;
    int x1 = (pos0.x + 1) % gradientTableSize.x;
    int y0 = pos0.y % gradientTableSize.y;
    int y1 = (pos0.y + 1) % gradientTableSize.y;

    float v00 = glm::dot((weights - glm::vec2(0.0f, 0.0f)), gradientTable[x0 + gradientTableSize.x * y0]);
    float v10 = glm::dot((weights - glm::vec2(1.0f, 0.0f)), gradientTable[x1 + gradientTableSize.x * y0]);
    float v01 = glm::dot((weights - glm::vec2(0.0f, 1.0f)), gradientTable[x0 + gradientTableSize.x * y1]);
    float v11 = glm::dot((weights - glm::vec2(1.0f, 1.0f)), gradientTable[x1 + gradientTableSize.x * y1]);

    //weights = glm::smoothstep(0.0f, 1.0f, weights); //Causes discontinuity at grid cell borders
    //6x^5 - 15x^4 + 10x^3 = x^3 * (x * (6x - 15) + 10) - fixes discontinuity
    weights = weights * weights * weights * (weights * (weights * 6.0f - 15.0f) + 10.0f);

    return glm::mix(glm::mix(v00, v10, weights.x), glm::mix(v01, v11, weights.x), weights.y);
}

float *GeneratePerlinNoise(glm::vec2 size, glm::ivec2 gridSize, int octaves, float persistence, float lacunarity)
{
    float *noise = (float *)calloc((int)(size.x * size.y), sizeof(float));

    glm::ivec2 gradientTableSize = glm::ivec2(256);
    glm::vec2 *gradientTable = (glm::vec2 *)calloc(gradientTableSize.x * gradientTableSize.y, sizeof(glm::vec2));
    for(int y = 0; y < gradientTableSize.y; y++)
    {
        for(int x = 0; x < gradientTableSize.x; x++)
        {
            glm::vec2 randomDirection = glm::vec2((float)rand() / RAND_MAX, (float)rand() / RAND_MAX) * 2.0f - 1.0f;
            gradientTable[x + gradientTableSize.x * y] = glm::normalize(randomDirection);
        }
    }

    for(int y = 0; y < size.y; y++)
    {
        for(int x = 0; x < size.x; x++)
        {
            float total = 0.0f;
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float maxAmplitude = 0.0f;

            for(int i = 0; i < octaves; i++)
            {
                glm::vec2 p = (glm::vec2(x, y) / size) * glm::vec2(gridSize) * frequency;

                if (i == 0)
                {
                    total += SamplePerlin(p, gradientTable, gradientTableSize);
                } else
                {
                    total += SamplePerlin(p, gradientTable, gradientTableSize) * amplitude;
                }

                amplitude *= persistence;
                frequency *= lacunarity;
            }

            //The largest distance vector from pos to grid cell corner is (1, 1) which has length sqrt(2)= +-1.414
            //Gradient vector is normalized and has length of 1
            //Thus the dot product between the two is in range +-1.414
            //The blend between the four dot products can at most be +-0.707 right at the center of the grid cell (0.5, 0.5)
            float normalizedValue = glm::clamp(((total / 0.707f) + 1.0f) / 2.0f, 0.0f, 1.0f);

            noise[x + (int)size.x * y] = normalizedValue;
        }
    }

    free(gradientTable);
    return noise;
}