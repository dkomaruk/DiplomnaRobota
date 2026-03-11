#ifndef CAMERA_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Game;

struct Camera
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float fov = 45.0f;

    glm::vec2 maxPitch = glm::vec2(-89.0f, 89.0f);

    float speed;
    float sensitivity;
};

Camera CreateFPSCamera();
void UpdateFPSCamera(Game *game);

#define CAMERA_H
#endif