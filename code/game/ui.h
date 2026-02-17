#ifndef UI_H

struct Game;

#include <GL/glew.h>

#include <glm/vec2.hpp>

void RenderRectUI(Game *game, glm::vec2 pos, glm::vec2 size, GLuint shader);

#define UI_H
#endif