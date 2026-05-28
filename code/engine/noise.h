#ifndef NOISE_H

#include "defines.h"

#include <glm/vec2.hpp>

u8 *NoiseToImage(float *noise, glm::vec2 size);

//Hand-coded version
float *GeneratePerlinNoise(glm::vec2 size, glm::ivec2 gridSize = glm::ivec2(32), int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f);

//More reliable noise generation using a helper library
float *GeneratePerlinNoise2(glm::vec2 size, glm::ivec2 gridSize = glm::ivec2(32), int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f);

#define NOISE_H
#endif