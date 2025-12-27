#include "text.h"

#include "mesh.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

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
        //NOTE: Increasing texture makes text quality worse because resolution is too low -> (vec2(w, h) * 2.0f)
        //It's better to load fonts with different sizes and use them when needed or use SDF
        *size = vec2(textSurface->w, textSurface->h);
    }

    GLuint texture = CreateGLTexture((uint8 *)textSurface->pixels, textSurface->pitch / 4, textSurface->h);

    SDL_DestroySurface(textSurface);

    return texture;
}

Text CreateText(Game *game, char *text, vec2 position, GLuint shader, int fontSize)
{
    Text result = {};

    result.shader = shader;
    result.position = position;
    result.texture = CreateTextureWithText(game, text, fontSize, &result.size);

    return result;
}

void DeleteText(Text *text)
{
    glDeleteTextures(1, &text->texture);
}

void RenderText(Game *game, Text *text)
{
    Mesh unitQuad = GetUnitQuad();

    glUseProgram(text->shader);

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
