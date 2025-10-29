#ifndef MESH_H

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct Game;

struct MaterialPhong
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float shininess;
};

struct Mesh
{
    GLuint vao, vbo;

    GLuint shader;
    GLuint texture;
    MaterialPhong material;

    int indicesCount, verticesCount;
};

Mesh CreateMesh(float *vertices, int verticesTotalSize, GLuint shader);
Mesh CreateMesh(float *vertices, int verticesTotalSize, unsigned int *indices, int indicesTotalSize, GLuint shader);

void RenderMesh(Game *game, Mesh *mesh, glm::mat4 model);

#define MESH_H
#endif