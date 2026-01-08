#include "text.h"

#include "texture.h"
#include "shader.h"
#include "image.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#define VERTICES_PER_CHARACTER 6

//Loads TTF font, saves glyph metrics, creates an atlas texture and uploads it to GPU
Font PrepareFont(char *filepath, int fontSize)
{
    Font font = {};

    font.ttfFont = TTF_OpenFont(filepath, (float)fontSize);

    int atlasWidth = 512;
    int atlasHeight = 512;

    SDL_Surface *atlas = SDL_CreateSurface(atlasWidth, atlasHeight, SDL_PIXELFORMAT_ARGB32);
    SDL_Surface *atlasSDF = SDL_CreateSurface(atlasWidth, atlasHeight, SDL_PIXELFORMAT_ARGB32);

    glm::ivec2 pos = glm::ivec2(0);
    int rowHeight = 0;

    for(char c = 32; c < 127; c++)
    {
        SDL_Surface *glyph = TTF_RenderGlyph_Blended(font.ttfFont, c, SDL_Color{255, 255, 255, 255});
        if(!glyph)
        {
            continue;
        }

        int minX, minY, maxX, maxY, advance;
        TTF_GetGlyphMetrics(font.ttfFont, c, &minX, &maxX, &minY, &maxY, &advance);

        Character ch = {};
        ch.bearing = glm::ivec2(minX, maxY);
        ch.characterSize = glm::ivec2(maxX - minX, maxY - minY);
        ch.textureSize = glm::ivec2(glyph->w, glyph->h);
        ch.advance = advance;

        if(int(pos.x + ch.advance) > atlasWidth)
        {
            pos.x = 0;
            pos.y += rowHeight + 1;

            rowHeight = 0;
        }

        ch.uvMin = glm::vec2((float)pos.x / atlasWidth, (float)pos.y / atlasHeight);
        ch.uvMax = glm::vec2((float)(pos.x + ch.advance) / atlasWidth, (float)(pos.y + glyph->h) / atlasHeight);

        font.characters[c] = ch;

        SDL_Rect srcRect = {0, 0, (int)ch.advance, glyph->h};
        SDL_Rect destRect = {pos.x, pos.y, (int)ch.advance, glyph->h};

        SDL_BlitSurface(glyph, &srcRect, atlas, &destRect);

        pos.x += ch.advance + 1;

        if(glyph->h > rowHeight)
        {
            rowHeight = glyph->h;
        }
    }

    font.atlas = CreateGLTexture((uint8 *)atlas->pixels, atlasWidth, atlasHeight);

    return font;
}

/* Creates a VBO that contains quads for all characters in the provided text.
 *
 * Each character is a quad with 6 vertices.
 * Each vertex has XY position, calculated from glyph metrics, and atlas texture UV coordinates. */
std::vector<VertexText> PrepareTextVertices(Font *font, char *text, glm::ivec2 *size = 0)
{
    std::vector<VertexText> vertices;

    glm::ivec2 nextPos = glm::ivec2(0);

    char c;
    int i = 0;
    while((c = text[i]) != '\0')
    {
        Character ch = font->characters[c];

        int xOffset = (ch.bearing.x < 0) ? ch.bearing.x : 0;
        if((nextPos.x + xOffset + ch.advance) > WINDOW_WIDTH - 10)
        {
            nextPos.x = 0;
            nextPos.y += ch.textureSize.y;
        }

        glm::ivec2 chPos = glm::ivec2(nextPos.x + xOffset, nextPos.y);

        VertexText v1 = {chPos, ch.uvMin};
        VertexText v2 = {glm::ivec2(chPos.x, chPos.y + ch.textureSize.y), glm::vec2(ch.uvMin.x, ch.uvMax.y)};
        VertexText v3 = {glm::ivec2(chPos.x + ch.advance, chPos.y + ch.textureSize.y), ch.uvMax};
        VertexText v4 = {glm::ivec2(chPos.x + ch.advance, chPos.y), glm::vec2(ch.uvMax.x, ch.uvMin.y)};

        vertices.insert(vertices.end(), {v1, v2, v3, v3, v4, v1});

        nextPos.x += ch.advance;
        i++;
    }

    *size = nextPos;

    return vertices;
}

Text CreateText(Font *font, char *text, glm::vec2 position, GLuint shader, glm::vec3 color)
{
    Text result = {};

    result.shader = shader;
    result.color = color;
    result.font = font;
    result.position = position;

    std::vector<VertexText> vertices =  PrepareTextVertices(font, text, &result.size);
    result.quads = CreateTextMesh(vertices);

    result.capacity = (int)vertices.size() / VERTICES_PER_CHARACTER;

    return result;
}

//Deletes text mesh data on the GPU
void DeleteText(Text *text)
{
    text->capacity = 0;
    text->quads.verticesCount = 0;

    glDeleteBuffers(1, &text->quads.vbo);
    glDeleteVertexArrays(1, &text->quads.vao);
}

//TODO: Write AppendText function

/* Updates text instance with new text.
 *
 * Old text is overwritten and text mesh data is recreated again.
 * If new text size is larger than current capacity, then VBO is resized to contain twice as many characters. */
void UpdateText(Text *text, char *newText, int length)
{
    char *temp = newText;
    if(length)
    {
        temp = (char *)malloc(length + 1);
        memcpy_s(temp, length, newText, length);
        temp[length] = '\0';
    }

    std::vector<VertexText> vertices = PrepareTextVertices(text->font, temp, &text->size);
    text->quads.verticesCount = (int)vertices.size();

    if(length)
    {
        free(temp);
    }

    glBindBuffer(GL_ARRAY_BUFFER, text->quads.vbo);

    int verticesSize = (int)(vertices.size() * sizeof(VertexText));
    int charactersCount = (int)vertices.size() / VERTICES_PER_CHARACTER;

    if(charactersCount > text->capacity)
    {
        text->capacity = charactersCount * 2;
        glBufferData(GL_ARRAY_BUFFER, verticesSize * 2, NULL, GL_DYNAMIC_DRAW);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, &vertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//Renders text. Model matrix is applied to text mesh in order to place it at text.position
void RenderText(Text *text)
{
    glUseProgram(text->shader);

    ShaderSetVec3(text->shader, "u_textColor", text->color);

    glm::mat4 model = glm::mat4(1.0f);
    model = translate(model, glm::vec3(text->position, 0.0f));
    //model = scale(model, glm::vec3(text->scale, 0.0f));
    ShaderSetMatrix4(text->shader, "u_model", model);

    ShaderSetInt(text->shader, "u_texture", 0);
    SetTexture(text->font->atlas, 0);
    glBindVertexArray(text->quads.vao);

    //TODO: Make a single draw call for all visible text (for each font)
    glDrawArrays(GL_TRIANGLES, 0, text->quads.verticesCount);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
