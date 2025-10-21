#ifndef MESH_H

#include <GL/glew.h>

struct Mesh
{
    GLuint vao, vbo;
    GLuint shader;

    GLuint texture;

    int indicesCount, verticesCount;
};

Mesh CreateMesh(float *vertices, int verticesTotalSize, GLuint shader);
Mesh CreateMesh(float *vertices, int verticesTotalSize, unsigned int *indices, int indicesTotalSize, GLuint shader);

#define MESH_H
#endif