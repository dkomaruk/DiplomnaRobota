#include "input.h"

#include "defines.h"

#include "game.h"

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_sdl3.h>

void ProcessInput(Game *game)
{
    Camera &camera = game->camera;

    memcpy(game->prevKeys, game->keys, SDL_SCANCODE_COUNT * sizeof(int));
    memcpy(game->prevMouseButtons, game->mouseButtons, MOUSE_BUTTONS_COUNT * sizeof(int));

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch(event.type)
        {
            case SDL_EVENT_QUIT:
            {
                game->isRunning = false;
            } break;

            case SDL_EVENT_KEY_UP:
            {
                if(event.key.repeat != 0) return;
                game->keys[event.key.scancode] = 0;
            } break;

            case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.scancode == SDL_SCANCODE_BACKSPACE && game->textDemo.typingText)
                {
                    game->textDemo.textChanged = true;
                    game->textDemo.textInputBuffer.pop_back();
                }

                if(event.key.repeat != 0) return;
                game->keys[event.key.scancode] = 1;
            } break;

            case SDL_EVENT_TEXT_INPUT:
            {
                game->textDemo.textChanged = true;
                game->textDemo.textInputBuffer += (char *)event.text.text;
            } break;

            case SDL_EVENT_MOUSE_MOTION:
            {
                if(game->isCursorHidden)
                {
                    SDL_MouseMotionEvent mouse = event.motion;

                    camera.yaw += mouse.xrel * camera.sensitivity;
                    camera.pitch -= mouse.yrel * camera.sensitivity;
                    camera.pitch = SDL_clamp(camera.pitch, camera.maxPitch.x, camera.maxPitch.y);

                    camera.direction.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
                    camera.direction.y = sin(glm::radians(camera.pitch));
                    camera.direction.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
                    camera.direction = normalize(camera.direction);
                }
            } break;

            case SDL_EVENT_MOUSE_WHEEL:
            {
                SDL_MouseWheelEvent wheel = event.wheel;

                camera.fov -= wheel.y;
                camera.fov = SDL_clamp(camera.fov, 1.0f, 45.0f);

                game->perspectiveProjection = glm::perspective(glm::radians(camera.fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
            } break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                game->mouseButtons[event.button.button - 1] = 1;
            } break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                game->mouseButtons[event.button.button - 1] = 0;
            } break;
        }
    }
}

bool IsFirstPress(Game *game, SDL_Scancode key)
{
    return game->keys[key] && !game->prevKeys[key];
}

bool IsFirstClick(Game *game, uint32 button)
{
    Assert(button < MOUSE_BUTTONS_COUNT)
    return game->mouseButtons[button] && !game->prevMouseButtons[button];
}

bool IsMouseJustReleased(Game *game, SDL_MouseButtonFlags button)
{
    Assert(button < MOUSE_BUTTONS_COUNT)
    return !game->mouseButtons[button] && game->prevMouseButtons[button];
}

char *GetMouseButtonName(int button)
{
    Assert(button < MOUSE_BUTTONS_COUNT);

    if(button == MOUSE_LEFT)
    {
        return "MOUSE_LEFT";
    }
    else if(button == MOUSE_RIGHT)
    {
        return "MOUSE_RIGHT";
    }
    else if(button == MOUSE_MIDDLE)
    {
        return "MOUSE_MIDDLE";
    }
    else if(button == MOUSE_SIDE1)
    {
        return "MOUSE_SIDE1";
    }
    else
    {
        return "MOUSE_SIDE2";
    }
}
