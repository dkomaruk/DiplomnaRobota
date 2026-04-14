#ifndef DEFINES_H

#include <stdint.h>
#include <glm/vec2.hpp>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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
#define RECT_HALF(rect) (glm::vec2((rect).x / 2.0f, (rect).y / 2.0f))
#define RECT_ASPECT_RATIO(rect) ((rect).x > (rect).y) ? ((float)(rect).x / (rect).y) : ((float)(rect).y / (rect).x)

#define DEFINES_H
#endif