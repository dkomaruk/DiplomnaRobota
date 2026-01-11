#ifndef RANDOM_H

#include <stdlib.h>

inline float RandomBetween(float min, float max)
{
    if(min == max) return max;
    return min + (((float)rand() / RAND_MAX) * (max - min));
}


#define RANDOM_H
#endif