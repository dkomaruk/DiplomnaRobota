#ifndef STRING_UTILS_H

#include <stdlib.h>

inline int StrToInt(const char *s)
{
    return (int)strtol(s, NULL, 10);
}

inline float StrToFloat(const char *s)
{
    return (float)strtol(s, NULL, 10);
}

#define STRING_UTILS_H
#endif