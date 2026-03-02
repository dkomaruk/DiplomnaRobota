#ifndef DEFINES_H

#include <stdint.h>
#include <glm/vec2.hpp>

#if defined(WINDOW_TRANSPARENT) || defined(WINDOW_BORDERLESS)
    #define WINDOW_WIDTH 1920.0f
    #define WINDOW_HEIGHT 1080.0f
#else
    #define WINDOW_WIDTH 1280.0f
    #define WINDOW_HEIGHT 720.0f
    //#define WINDOW_WIDTH 1920.0f
    //#define WINDOW_HEIGHT 1080.0f
#endif

#define WINDOW_CENTER glm::vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f)

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define Assert(expression) \
        if(!(expression)) { *(int *)0 = 0; }

#define InvalidCodepath Assert(0);
#define CaseNotImplemented default: {InvalidCodepath} break;

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define FLAG_SET(n, f) ((n) |= (f))
#define FLAG_CLEAR(n, f) ((n) &= ~(f))
#define FLAG_TOGGLE(n, f) ((n) ^= (f))
#define FLAG_IS_SET(n, f) (((n) & (f)) == (f))
#define FLAG_IS_SINGLE(f) (!(f) || ((f) & ((f) - 1)))

#define RECT_HAS_SIZE(rectSize) (glm::abs((rectSize).x) > 0 && glm::abs((rectSize).y) > 0)

#define DEFINES_H
#endif