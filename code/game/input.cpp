#include "input.h"

#include "defines.h"

#include "game.h"

#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_sdl3.h>

void ProcessInput(Input *input)
{
    memcpy(input->prevKeys, input->keys, SDL_SCANCODE_COUNT * sizeof(int));
    memcpy(input->prevMouseButtons, input->mouseButtons, MOUSE_BUTTONS_COUNT * sizeof(int));

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch(event.type)
        {
            case SDL_EVENT_QUIT:
            {
                input->shouldQuit;
            } break;

            case SDL_EVENT_KEY_UP:
            {
                input->keys[event.key.scancode] = KEY_UP;
            } break;

            case SDL_EVENT_KEY_DOWN:
            {
                if(event.key.scancode == SDL_SCANCODE_BACKSPACE)
                    input->isBackspacePressed = true;

                if(event.key.repeat != 0) break;
                input->keys[event.key.scancode] = KEY_DOWN;
            } break;

            case SDL_EVENT_TEXT_INPUT:
            {
                input->typedText += event.text.text;
            } break;

            case SDL_EVENT_MOUSE_MOTION:
            {
                input->mousePos = glm::vec2(event.motion.x, event.motion.y);
                input->mouseDelta = glm::vec2(event.motion.xrel, event.motion.yrel);
            } break;

            case SDL_EVENT_MOUSE_WHEEL:
            {
                input->mouseWheelDelta = glm::vec2(event.wheel.x, event.wheel.y);
            } break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                input->mouseButtons[event.button.button - 1] = 1;
            } break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                input->mouseButtons[event.button.button - 1] = 0;
            } break;
        }
    }

    input->isMouseCapturedByImgui = ImGui::GetIO().WantCaptureMouse;
}

bool IsFirstPress(Game *game, SDL_Scancode key)
{
    return game->input.keys[key] && !game->input.prevKeys[key];
}

bool IsFirstClick(Game *game, uint32 button)
{
    Assert(button < MOUSE_BUTTONS_COUNT)
    return game->input.mouseButtons[button] && !game->input.prevMouseButtons[button];
}

bool IsMouseJustReleased(Game *game, uint32 button)
{
    Assert(button < MOUSE_BUTTONS_COUNT)
    return !game->input.mouseButtons[button] && game->input.prevMouseButtons[button];
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
