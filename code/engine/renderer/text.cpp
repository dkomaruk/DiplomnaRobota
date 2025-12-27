#include "text.h"

#include "mesh.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

Text CreateText(vec2 position, vec2 size, GLuint texture, GLuint shader)
{
    Text text = {};

    text.quad = CreateQuad(position, size);
    text.texture = texture;
    text.shader = shader;

    return text;
}

void RenderText(Game *game, Text *text)
{
    glUseProgram(text->shader);

    mat4 model = mat4(1.0f);
    ShaderSetMatrix4(text->shader, "u_model", model);

    SetTexture(text->texture, 0);
    glBindVertexArray(text->quad.vao);

    glDrawArrays(GL_TRIANGLES, 0, text->quad.verticesCount);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}