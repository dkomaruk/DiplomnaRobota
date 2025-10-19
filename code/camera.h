#ifndef CAMERA_H

#include <glm/vec3.hpp>

struct Camera
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;

    float speed;
};

#define CAMERA_H
#endif