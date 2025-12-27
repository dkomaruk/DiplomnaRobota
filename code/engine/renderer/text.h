#ifndef TEXT_H

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "mesh.h"

struct Text
{
    Mesh quad;

    GLuint texture;
    GLuint shader;

    vec2 offset;
};

Text CreateText(vec2 position, vec2 size, GLuint texture, GLuint shader);
void RenderText(Game *game, Text *text);

#define TEXT_H
#endif