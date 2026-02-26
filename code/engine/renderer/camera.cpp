#include "camera.h"

#include "game.h"
#include "input.h"

void UpdateCamera(Game *game)
{
    Input *input = &game->input;
    Camera *camera = &game->camera;

    glm::vec3 dir = camera->direction;
    //dir.y = 0.0f;
    dir = normalize(dir);

    float cameraSpeed = camera->speed * game->deltaTime;
    if(input->keys[SDL_SCANCODE_LSHIFT])
    {
        cameraSpeed *= 5.0f;
    }

    //Camera orientation
    if(input->isCursorHidden)
    {
        camera->yaw += input->mouseDelta.x * camera->sensitivity;
        camera->pitch -= input->mouseDelta.y * camera->sensitivity;
        camera->pitch = SDL_clamp(camera->pitch, camera->maxPitch.x, camera->maxPitch.y);

        camera->direction.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        camera->direction.y = sin(glm::radians(camera->pitch));
        camera->direction.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        camera->direction = normalize(camera->direction);
    }

#if 0
    //Camera zoom
    camera->fov -= input->mouseWheelDelta.y;
    camera->fov = SDL_clamp(camera->fov, 1.0f, 45.0f);

    game->perspectiveProjection = glm::perspective(glm::radians(camera->fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
#endif

    //Camera movement
    if(input->keys[SDL_SCANCODE_W])
        camera->position += cameraSpeed * dir;
    if(input->keys[SDL_SCANCODE_S])
        camera->position -= cameraSpeed * dir;
    if(input->keys[SDL_SCANCODE_A])
        camera->position -= normalize(cross(dir, camera->up)) * cameraSpeed;
    if(input->keys[SDL_SCANCODE_D])
        camera->position += normalize(cross(dir, camera->up)) * cameraSpeed;

    //camera->position.y = 1.0f;

    game->view = lookAt(camera->position, camera->position + camera->direction, glm::vec3(0.0f, 1.0f, 0.0f));
}


