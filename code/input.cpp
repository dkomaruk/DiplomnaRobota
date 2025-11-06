#include "input.h"

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

void ProcessKeyUp(SDL_KeyboardEvent *event, Game *game)
{
    if(event->repeat != 0) return;
    game->keys[event->scancode] = 0;
}

void ProcessKeyDown(SDL_KeyboardEvent *event, Game *game)
{
    if(event->repeat != 0) return;
    game->keys[event->scancode] = 1;
}

void ProcessInput(Game *game)
{
    Camera &camera = game->camera;

    memcpy(game->prevKeys, game->keys, SDL_SCANCODE_COUNT * sizeof(int));

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_EVENT_QUIT:
            {
                game->isRunning = false;
            } break;

            case SDL_EVENT_KEY_UP:
            {
                ProcessKeyUp(&event.key, game);
            } break;

            case SDL_EVENT_KEY_DOWN:
            {
                ProcessKeyDown(&event.key, game);
            } break;

            case SDL_EVENT_MOUSE_MOTION:
            {
                SDL_MouseMotionEvent mouse = event.motion;

                camera.yaw += mouse.xrel * camera.sensitivity;
                camera.pitch -= mouse.yrel * camera.sensitivity;
                camera.pitch = SDL_clamp(camera.pitch, camera.maxPitch.x, camera.maxPitch.y);

                camera.direction.x = cos(radians(camera.yaw)) * cos(radians(camera.pitch));
                camera.direction.y = sin(radians(camera.pitch));
                camera.direction.z = sin(radians(camera.yaw)) * cos(radians(camera.pitch));
                camera.direction = normalize(camera.direction);
            } break;

            case SDL_EVENT_MOUSE_WHEEL:
            {
                SDL_MouseWheelEvent wheel = event.wheel;

                camera.fov -= wheel.y;
                camera.fov = SDL_clamp(camera.fov, 1.0f, 45.0f);

                game->projection = perspective(radians(camera.fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
            } break;

        }
    }
}