#ifndef TEXT_H

#include "mesh.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include <map>

struct Character
{
    vec2 uvMin, uvMax;
    ivec2 bearing, size;
    ivec2 textureSize;
    uint32 advance;
};

struct Font
{
    GLuint atlas;
    TTF_Font *ttfFont;

    GLuint atlasSDF;
    TTF_Font *ttfFontSDF;

    std::map<char, Character> characters;
};

struct Text
{
    Font *font;

    vec3 color;

    Mesh quads;
    GLuint shader;

    vec2 position;
    ivec2 size;

    //Size & capacity in characters (6 vertices per one character)
    int count;
    int capacity;
};

Font PrepareFont(char *filepath, int fontSize);

Text CreateText(Font *font, char *text, vec2 position, GLuint shader, vec3 color = vec3(1.0f));
void UpdateText(Text *text, char *newText, int length = 0);
void DeleteText(Text *text);
void RenderText(Text *text);

#define TEXT_H
#endif