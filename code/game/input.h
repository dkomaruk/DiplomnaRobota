#ifndef INPUT_H

#include "defines.h"

#include <SDL3/SDL.h>

#define MOUSE_BUTTONS_COUNT 5
#define MOUSE_LEFT 0
#define MOUSE_MIDDLE 1
#define MOUSE_RIGHT 2
#define MOUSE_SIDE1 3
#define MOUSE_SIDE2 4

struct Game;

void ProcessInput(Game *game);

bool IsFirstPress(Game *game, SDL_Scancode key);

bool IsFirstClick(Game *game, uint32 button);
bool IsMouseJustReleased(Game *game, uint32 button);
char *GetMouseButtonName(int button);

#define INPUT_H
#endif