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
    int ascent, descent;

    std::map<char, Character> characters;
};

struct DynamicText
{
    Font *font;

    vec3 color;

    Mesh quads;
    GLuint shader;

    vec2 position;

    //Size & capacity in characters (6 vertices per one character)
    int size;
    int capacity;
};

struct StaticText
{
    GLuint texture, shader;
    vec2 position, size;

    vec3 color;
};

Font PrepareFont(char *filepath, int fontSize);

DynamicText CreateDynamicText(Font *font, char *text, vec2 position, GLuint shader);
void UpdateDynamicText(DynamicText *text, char *newText);
void DeleteDynamicText(DynamicText *text);
void RenderDynamicText(DynamicText *text);

StaticText CreateStaticText(Game *game, char *text, vec2 position, GLuint shader, int fontSize = 18);
void DeleteStaticText(StaticText *text);
void RenderStaticText(StaticText *text);

#define TEXT_H
#endif