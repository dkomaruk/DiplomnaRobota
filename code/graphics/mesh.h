#ifndef MESH_H

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

struct Game;

struct MaterialPhong
{
    GLuint diffuseTexture; //GL_TEXTURE0
    GLuint specularTexture; //GL_TEXTURE1
    GLuint emissionTexture; //GL_TEXTURE2

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

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
void RenderMesh(Game *game, Mesh *mesh, glm::mat4 model);

#define MESH_H
#endif