#include "text.h"

#include "texture.h"
#include "image.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#define VERTICES_PER_CHARACTER 6

Font PrepareFont(char *filepath, int fontSize)
{
    Font font = {};

    font.ttfFont = TTF_OpenFont(filepath, (float)fontSize);

    font.ttfFontSDF = TTF_OpenFont(filepath, (float)fontSize);
    TTF_SetFontSDF(font.ttfFontSDF, true);

    int atlasWidth = 512;
    int atlasHeight = 512;

    SDL_Surface *atlas = SDL_CreateSurface(atlasWidth, atlasHeight, SDL_PIXELFORMAT_ARGB32);
    SDL_Surface *atlasSDF = SDL_CreateSurface(atlasWidth, atlasHeight, SDL_PIXELFORMAT_ARGB32);

    ivec2 pos = ivec2(0);
    int rowHeight = 0;

    for(char c = 32; c < 127; c++)
    {
        SDL_Surface *glyph = TTF_RenderGlyph_Blended(font.ttfFont, c, SDL_Color{255, 255, 255, 255});
        SDL_Surface *glyphSDF = TTF_RenderGlyph_Blended(font.ttfFontSDF, c, SDL_Color{255, 255, 255, 255});

        if(!glyph || !glyphSDF) continue;

        int minX, minY, maxX, maxY, advance;
        TTF_GetGlyphMetrics(font.ttfFont, c, &minX, &maxX, &minY, &maxY, &advance);

        Character ch = {};
        ch.textureSize = ivec2(glyph->w, glyph->h);
        ch.bearing = ivec2(minX, maxY);
        ch.size = ivec2(maxX - minX, maxY - minY);
        ch.advance = advance;

        if(int(pos.x + ch.advance) > atlasWidth)
        {
            pos.x = 0;
            pos.y += rowHeight + 1;

            rowHeight = 0;
        }

        ch.uvMin = vec2((float)pos.x / atlasWidth, (float)pos.y / atlasHeight);
        ch.uvMax = vec2((float)(pos.x + ch.advance) / atlasWidth, (float)(pos.y + glyph->h) / atlasHeight);

        font.characters[c] = ch;

        SDL_Rect srcRect = {0, 0, (int)ch.advance, glyph->h};
        SDL_Rect destRect = {pos.x, pos.y, (int)ch.advance, glyph->h};

        SDL_BlitSurface(glyph, &srcRect, atlas, &destRect);
        SDL_BlitSurface(glyphSDF, &srcRect, atlasSDF, &destRect);

        pos.x += ch.advance + 1;

        if(glyph->h > rowHeight)
        {
            rowHeight = glyph->h;
        }
    }

    font.atlas = CreateGLTexture((uint8 *)atlas->pixels, atlasWidth, atlasHeight);
    font.atlasSDF = CreateGLTexture((uint8 *)atlasSDF->pixels, atlasWidth, atlasHeight);

    //SaveImage("atlas.jpg", atlas->pixels, 512, 512, 4, false);

    return font;
}

std::vector<VertexText> PrepareTextVertices(Font *font, char *text, ivec2 *size = 0)
{
    std::vector<VertexText> vertices;

    ivec2 nextPos = ivec2(0);

    char c;
    int i = 0;
    while((c = text[i]) != '\0')
    {
        Character ch = font->characters[c];

        if(nextPos.x + ch.advance > WINDOW_WIDTH)
        {
            nextPos.x = 0;
            nextPos.y += ch.textureSize.y;
        }

        int xOffset = (ch.bearing.x < 0) ? ch.bearing.x : 0;
        ivec2 chPos = ivec2(nextPos.x + xOffset, nextPos.y);

        VertexText v1 = {chPos, ch.uvMin};
        VertexText v2 = {ivec2(chPos.x, chPos.y + ch.textureSize.y), vec2(ch.uvMin.x, ch.uvMax.y)};
        VertexText v3 = {ivec2(chPos.x + ch.advance, chPos.y + ch.textureSize.y), ch.uvMax};
        VertexText v4 = {ivec2(chPos.x + ch.advance, chPos.y), vec2(ch.uvMax.x, ch.uvMin.y)};

        vertices.insert(vertices.end(), {v1, v2, v3, v3, v4, v1});

        nextPos.x += ch.advance;
        i++;
    }

    *size = nextPos;

    return vertices;
}

Text CreateText(Font *font, char *text, vec2 position, GLuint shader, vec3 color)
{
    Text result = {};

    result.shader = shader;
    result.color = color;
    result.font = font;
    result.position = position;

    std::vector<VertexText> vertices =  PrepareTextVertices(font, text, &result.size);
    result.quads = CreateTextMesh(vertices);

    result.count = result.capacity = (int)vertices.size() / VERTICES_PER_CHARACTER;

    return result;
}

void DeleteText(Text *text)
{
    glDeleteBuffers(1, &text->quads.vbo);
    glDeleteVertexArrays(1, &text->quads.vao);
}

void UpdateText(Text *text, char *newText, int length)
{
    char *temp;
    if(length)
    {
        temp = (char *)malloc(length + 1);
        memcpy_s(temp, length, newText, length);
        temp[length] = '\0';
    }
    else
    {
        temp = newText;
    }

    std::vector<VertexText> vertices = PrepareTextVertices(text->font, temp, &text->size);
    int verticesSize = (int)(vertices.size() * sizeof(VertexText));

    if(length)
        free(temp);

    glBindBuffer(GL_ARRAY_BUFFER, text->quads.vbo);

    int newSize = (int)vertices.size() / VERTICES_PER_CHARACTER;
    if(newSize > text->capacity)
    {
        text->capacity = newSize * 2;
        //glBufferData(GL_ARRAY_BUFFER, verticesSize * 2, &vertices[0], GL_DYNAMIC_DRAW); -> crashes sometimes
        glBufferData(GL_ARRAY_BUFFER, verticesSize * 2, NULL, GL_DYNAMIC_DRAW);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, &vertices[0]);

    text->count = newSize;
    text->quads.verticesCount = (int)vertices.size();
}

void RenderText(Text *text)
{
    glUseProgram(text->shader);

    ShaderSetVec3(text->shader, "u_textColor", text->color);

    mat4 model = mat4(1.0f);
    model = translate(model, vec3(text->position, 0.0f));
    //model = scale(model, vec3(text->scale, 0.0f));
    ShaderSetMatrix4(text->shader, "u_model", model);

    ShaderSetInt(text->shader, "u_texture", 0);
    SetTexture(text->font->atlas, 0);
    glBindVertexArray(text->quads.vao);

    //TODO: Make a single draw call for all visible dynamic text
    glDrawArrays(GL_TRIANGLES, 0, text->quads.verticesCount);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
