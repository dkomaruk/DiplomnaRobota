#ifndef MATH_UTILS_H

inline float Clamp(float min, float value, float max)
{
    float result = value;

    if(result < min)
    {
        result = min;
    }
    else if(result > max)
    {
        result = max;
    }

    return(result);
}

inline float Clamp01(float value)
{
    float result = Clamp(0.0f, value, 1.0f);
    return(result);
}

inline float Clamp01MapToRange(float min, float t, float max)
{
    float result = 0.0f;

    float range = max - min;
    if(range != 0.0f)
    {
        result = Clamp01((t - min) / range);
    }

    return(result);
}

inline float Min(float a, float b)
{
    return (a < b) ? a : b;
}

inline int Min(int a, int b)
{
    return (a < b) ? a : b;
}

inline float Max(float a, float b)
{
    return (a > b) ? a : b;
}

inline int Max(int a, int b)
{
    return (a > b) ? a : b;
}


#define MATH_UTILS_H
#endif
