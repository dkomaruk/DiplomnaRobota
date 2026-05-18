#ifndef INPUT_H

#include "defines.h"

#include <SDL3/SDL.h>

#include <glm/vec2.hpp>

#include <string>

#define MOUSE_BUTTONS_COUNT 5
#define MOUSE_LEFT 0
#define MOUSE_MIDDLE 1
#define MOUSE_RIGHT 2
#define MOUSE_SIDE1 3
#define MOUSE_SIDE2 4

#define KEY_UP 0
#define KEY_DOWN 1

struct Input
{
    int prevKeys[SDL_SCANCODE_COUNT], keys[SDL_SCANCODE_COUNT];

    int mouseButtons[MOUSE_BUTTONS_COUNT], prevMouseButtons[MOUSE_BUTTONS_COUNT];
    bool isCursorHidden;
    bool isMouseCapturedByImgui;

    bool shouldQuit;

    glm::vec2 mousePos;
    glm::vec2 mouseDelta;
    glm::vec2 mouseWheelDelta;

    std::string typedText = "";
    bool isBackspacePressed;
};

void ProcessInput(Input *input);

bool IsFirstPress(Input *input, SDL_Scancode key);
bool IsKeyDown(Input *input, SDL_Scancode key);

bool IsFirstClick(Input *input, u32 button);
bool IsButtonDown(Input *input, u32 button);
bool IsMouseJustReleased(Input *input, u32 button);
char *GetMouseButtonName(int button);


#define INPUT_H
#endif