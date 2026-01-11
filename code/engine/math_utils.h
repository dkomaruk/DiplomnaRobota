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


#define MATH_UTILS_H
#endif
