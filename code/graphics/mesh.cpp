#include "mesh.h"

#include "texture.h"

#include "game.h"

Mesh InitVBO(float *vertices, int verticesTotalSize)
{
    Mesh mesh = {};

    //TODO: Variable vertex attributes
    int vertexAttribCount = 8;
    mesh.verticesCount = verticesTotalSize / (vertexAttribCount * sizeof(float));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, verticesTotalSize, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(0 * sizeof(float)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mesh;
}

Mesh CreateMesh(float *vertices, int verticesTotalSize, GLuint shader)
{
    Mesh mesh = InitVBO(vertices, verticesTotalSize);
    mesh.shader = shader;

    return mesh;
}

Mesh CreateMesh(float *vertices, int verticesTotalSize, unsigned int *indices, int indicesTotalSize, GLuint shader)
{
    Mesh mesh = InitVBO(vertices, verticesTotalSize);
    mesh.shader = shader;
    mesh.indicesCount = indicesTotalSize / sizeof(int);

    glBindVertexArray(mesh.vao);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesTotalSize, indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return mesh;
}

void RenderMesh(Game *game, Mesh *mesh, glm::mat4 model)
{
    GLuint shader = mesh->shader;
    UseShader(shader);

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(glGetUniformLocation(shader, "u_normalMatrix"), 1, GL_FALSE, value_ptr(normalMatrix));

    glUniformMatrix4fv(glGetUniformLocation(shader, "u_model"), 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_view"), 1, GL_FALSE, value_ptr(game->view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_projection"), 1, GL_FALSE, value_ptr(game->projection));

    MaterialPhong mat = mesh->material;
    ShaderSetVec3(shader, "u_material.diffuse", mat.diffuse);
    ShaderSetVec3(shader, "u_material.ambient", mat.ambient);
    ShaderSetVec3(shader, "u_material.specular", mat.specular);
    ShaderSetFloat(shader, "u_material.shininess", mat.shininess);

    if(mesh->texture)
    {
        SetTexture(mesh->texture, 0);
    }

    glBindVertexArray(mesh->vao);
    if(mesh->indicesCount > 0)
    {
        glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mesh->verticesCount);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}