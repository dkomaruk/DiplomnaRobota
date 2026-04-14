#ifndef TEXT_H

#include "mesh.h"

#include "defines.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include <map>

struct Character
{
    glm::vec2 uvMin, uvMax;

    glm::ivec2 bearing;
    glm::ivec2 characterSize;
    glm::ivec2 textureSize;

    u32 advance;
};

struct Font
{
    Texture atlas;
    TTF_Font *ttfFont;

    std::map<char, Character> characters;
};

struct Text
{
    Font *font;

    glm::vec3 color;

    Mesh quads;
    GLuint shader;

    glm::vec2 position;
    glm::ivec2 size;

    int capacity; //How many characters text can hold without having to allocate more memory on GPU
};

Font PrepareFont(char *filepath, int fontSize);

Text CreateText(Font *font, char *text, glm::vec2 position, GLuint shader, glm::vec3 color = glm::vec3(1.0f));
void UpdateText(Text *text, char *newText, int length = 0);
void DeleteText(Text *text);
void RenderText(Text *text);

#define TEXT_H
#endif