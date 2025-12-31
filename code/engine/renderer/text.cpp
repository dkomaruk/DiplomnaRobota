#include "text.h"

#include "texture.h"
#include <SDL3_ttf/SDL_ttf.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#define VERTICES_PER_CHARACTER 6

Font PrepareFont(char *filepath, int fontSize)
{
    Font font = {};

    font.ttfFont = TTF_OpenFont(filepath, (float)fontSize);

    font.ascent = TTF_GetFontAscent(font.ttfFont);
    font.descent = TTF_GetFontDescent(font.ttfFont);

    int atlasWidth = 1024;
    int atlasHeight = 1024;

    SDL_Surface *atlas = SDL_CreateSurface(atlasWidth, atlasHeight, SDL_PIXELFORMAT_ARGB32);

    ivec2 pos = ivec2(0);
    int rowHeight = 0;

    for(char c = 32; c < 127; c++)
    {
        SDL_Surface *glyph = TTF_RenderGlyph_Blended(font.ttfFont, c, SDL_Color{255, 255, 255, 255});
        if(!glyph) continue;

        int minX, minY, maxX, maxY, advance;
        TTF_GetGlyphMetrics(font.ttfFont, c, &minX, &maxX, &minY, &maxY, &advance);

        Character ch = {};
        ch.textureSize = ivec2(glyph->w, glyph->h);
        ch.bearing = ivec2(minX, maxY);
        ch.size = ivec2(maxX - minX, maxY - minY);
        ch.advance = advance;

        if(int(pos.x + ch.textureSize.x) > atlasWidth)
        {
            pos.x = 0;
            pos.y += rowHeight + 1;

            rowHeight = 0;
        }

        ch.uvMin = vec2((float)pos.x / atlasWidth, (float)pos.y / atlasHeight);
        ch.uvMax = vec2((float)(pos.x + ch.textureSize.x) / atlasWidth, (float)(pos.y + glyph->h) / atlasHeight);

        //if(c == 'A' || c == 'S' || c == 'p' || c == 'P' || c == 'j' || c == 'a' || c == 'I' || c == 's' || c == ' ' || c == 'u')
        if(c == 'T' || c == 'h' || c == 'P' || c == 'A' || c == 'j')
        {
            SDL_Log("%c. Glyph: (%d, %d), ch.size: (%d %d)\n", c, glyph->w, glyph->h, ch.size.x, ch.size.y);
            SDL_Log("   MinX: %d, MaxX: %d, MinY: %d, MaxY: %d, Advance: %d\n", minX, maxX, minY, maxY, ch.advance);
        }

        font.characters[c] = ch;

        SDL_Rect srcRect = {0, 0, (int)ch.textureSize.x, glyph->h};
        SDL_Rect destRect = {pos.x, pos.y, (int)ch.textureSize.x, glyph->h};
        SDL_BlitSurface(glyph, &srcRect, atlas, &destRect);

        pos.x += ch.textureSize.x + 1;

        if(glyph->h > rowHeight)
        {
            rowHeight = glyph->h;
        }
    }

    font.atlas = CreateGLTexture((uint8 *)atlas->pixels, atlasWidth, atlasHeight);

    stbi_flip_vertically_on_write(false);
    stbi_write_png("atlas.png", 1024, 1024, 4, atlas->pixels, 4 * 1024);
    stbi_flip_vertically_on_write(true);

    bool isKerningEnabled = TTF_GetFontKerning(font.ttfFont);

    int kerning;
    TTF_GetGlyphKerning(font.ttfFont, ' ', ')', &kerning);

    SDL_Log("Kearning: %d, 'A':'P' - %d\n", isKerningEnabled, kerning);

    return font;
}

std::vector<VertexText> PrepareTextVertices(Font *font, char *text, vec2 position)
{
    std::vector<VertexText> vertices;

    ivec2 nextPos = position;

    char c;
    int i = 0;
    while((c = text[i]) != '\0')
    {
        Character ch = font->characters[c];

        int xOffset = (ch.bearing.x < 0) ? ch.bearing.x : 0; //This is needed for negative bearing (minX)
        ivec2 chPos = ivec2(nextPos.x + xOffset, nextPos.y);

        VertexText v1 = {chPos, ch.uvMin};
        VertexText v2 = {ivec2(chPos.x, chPos.y + ch.textureSize.y), vec2(ch.uvMin.x, ch.uvMax.y)};
        VertexText v3 = {ivec2(chPos.x + ch.textureSize.x, chPos.y + ch.textureSize.y), ch.uvMax};
        VertexText v4 = {ivec2(chPos.x + ch.textureSize.x, chPos.y), vec2(ch.uvMax.x, ch.uvMin.y)};

        vertices.insert(vertices.end(), {v1, v2, v3, v3, v4, v1});

        nextPos.x += ch.advance;
        i++;
    }

    return vertices;
}

DynamicText CreateDynamicText(Font *font, char *text, vec2 position, GLuint shader)
{
    DynamicText result = {};

    result.shader = shader;
    result.font = font;
    result.position = position;

    std::vector<VertexText> vertices =  PrepareTextVertices(font, text, position);
    result.quads = CreateTextMesh(vertices);

    result.size = result.capacity = (int)vertices.size() / VERTICES_PER_CHARACTER;

    return result;
}

void DeleteDynamicText(DynamicText *text)
{
    glDeleteBuffers(1, &text->quads.vbo);
    glDeleteVertexArrays(1, &text->quads.vao);
}

void UpdateDynamicText(DynamicText *text, char *newText)
{
    std::vector<VertexText> vertices = PrepareTextVertices(text->font, newText, text->position);
    int verticesSize = (int)(vertices.size() * sizeof(VertexText));

    glBindBuffer(GL_ARRAY_BUFFER, text->quads.vbo);

    int newSize = (int)vertices.size() / VERTICES_PER_CHARACTER;
    if(newSize > text->capacity)
    {
        text->capacity = newSize * 2;
        glBufferData(GL_ARRAY_BUFFER, verticesSize * 2, &vertices[0], GL_DYNAMIC_DRAW);
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, &vertices[0]);
    }

    text->size = newSize;
    text->quads.verticesCount = (int)vertices.size();
}

void RenderDynamicText(DynamicText *text)
{
    glUseProgram(text->shader);

    ShaderSetVec3(text->shader, "u_textColor", text->color);

    mat4 model = mat4(1.0f);
    //model = translate(model, vec3(text->position, 0.0f));
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

GLuint CreateTextureWithText(Game *game, char *text, int fontSize = 18, vec2 *size = 0)
{
    if(!game->fonts.count(fontSize))
    {
        SDL_Log("Failed to create text from string \"%s\". Incorrect font size %d\n", text, fontSize);
        return 0;
    }

    SDL_Surface *textSurface = TTF_RenderText_Blended(game->fonts[fontSize], text, 0, SDL_Color{255, 255, 255, 255});
    SDL_FlipSurface(textSurface, SDL_FLIP_VERTICAL);

    if(size)
    {
        //NOTE: Increasing texture size makes text quality worse because resolution is too low -> (vec2(w, h) * 2.0f)
        //It's better to load fonts with different sizes and use them when needed or use SDF
        *size = vec2(textSurface->w, textSurface->h);
    }

    GLuint texture = CreateGLTexture((uint8 *)textSurface->pixels, textSurface->pitch / 4, textSurface->h);

    SDL_DestroySurface(textSurface);

    return texture;
}

StaticText CreateStaticText(Game *game, char *text, vec2 position, GLuint shader, int fontSize)
{
    StaticText result = {};

    result.shader = shader;
    result.position = position;
    result.texture = CreateTextureWithText(game, text, fontSize, &result.size);

    result.color = vec3(1.0f, 1.0f, 1.0f);

    return result;
}

void DeleteStaticText(StaticText *text)
{
    glDeleteTextures(1, &text->texture);
}

void RenderStaticText(StaticText *text)
{
    Mesh unitQuad = GetUnitQuad();

    glUseProgram(text->shader);

    ShaderSetVec3(text->shader, "u_textColor", text->color);

    mat4 model = mat4(1.0f);
    model = translate(model, vec3(text->position, 0.0f));
    model = scale(model, vec3(text->size, 0.0f));
    ShaderSetMatrix4(text->shader, "u_model", model);

    ShaderSetInt(text->shader, "u_texture", 0);
    SetTexture(text->texture, 0);
    glBindVertexArray(unitQuad.vao);

    glDrawArrays(GL_TRIANGLES, 0, unitQuad.verticesCount);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
