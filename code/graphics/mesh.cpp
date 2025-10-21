#include "mesh.h"

#include "texture.h"

Mesh InitVBO(float *vertices, int verticesTotalSize)
{
    Mesh mesh = {};

    //TODO: Variable vertex attributes
    int vertexAttribCount = 5;
    mesh.verticesCount = verticesTotalSize / (vertexAttribCount * sizeof(float));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, verticesTotalSize, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(0 * sizeof(float)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

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