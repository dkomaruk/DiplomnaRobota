#ifndef TEXT_H

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "mesh.h"

struct Text
{
    GLuint texture;
    GLuint shader;

    vec2 position;
    vec2 size;
};

Text CreateText(Game *game, char *text, vec2 position, GLuint shader, int fontSize = 18);
void DeleteText(Text *text);
void RenderText(Game *game, Text *text);

#define TEXT_H
#endif