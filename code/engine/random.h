#ifndef RANDOM_H

#include <glm/vec3.hpp>
#include <stdlib.h>

inline float RandomBetween(float min, float max)
{
    if(min == max) return max;
    return min + (((float)rand() / RAND_MAX) * (max - min));
}

inline glm::vec3 RandomBetween(glm::vec3 min, glm::vec3 max)
{
    if(min == max) return max;

    glm::vec3 result;
    result.x = RandomBetween(min.x, max.x);
    result.y = RandomBetween(min.y, max.y);
    result.z = RandomBetween(min.z, max.z);

    return result;
}

inline glm::vec4 RandomBetween(glm::vec4 min, glm::vec4 max)
{
    if(min == max) return max;

    glm::vec4 result;
    result.x = RandomBetween(min.x, max.x);
    result.y = RandomBetween(min.y, max.y);
    result.z = RandomBetween(min.z, max.z);
    result.w = RandomBetween(min.w, max.w);

    return result;
}


#define RANDOM_H
#endif