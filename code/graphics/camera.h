#ifndef CAMERA_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Camera
{
    vec3 position;
    vec3 direction;
    vec3 up;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float fov = 45.0f;

    vec2 maxPitch = vec2(-89.0f, 89.0f);

    float speed;
    float sensitivity;
};

#define CAMERA_H
#endif