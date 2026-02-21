#ifndef NOISE_H

#include "defines.h"

#include <glm/vec2.hpp>

uint8 *GenerateValueNoise(glm::vec2 size);
uint8* GeneratePerlinNoise(glm::vec2 size, glm::ivec2 gridSize = glm::ivec2(32), int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f);

#define NOISE_H
#endif