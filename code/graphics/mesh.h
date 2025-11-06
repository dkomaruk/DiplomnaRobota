#ifndef MESH_H

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <vector>

struct Game;

struct MaterialPhong
{
    GLuint diffuseTexture; //GL_TEXTURE0
    GLuint specularTexture; //GL_TEXTURE1
    GLuint emissionTexture; //GL_TEXTURE2

    float shininess;
};

struct Vertex
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

struct Mesh
{
    GLuint vao, vbo;

    GLuint shader;
    GLuint texture;
    MaterialPhong material;

    int indicesCount, verticesCount;
};

struct Model
{
    Mesh *meshes;
    int numOfMeshes;
};

Mesh CreateMesh(float *vertices, int verticesTotalSize, GLuint shader);
Mesh CreateMesh(float *vertices, int verticesTotalSize, unsigned int *indices, int indicesTotalSize, GLuint shader);

Mesh CreateMesh(std::vector<Vertex> vertices, GLuint shader);
Mesh CreateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, GLuint shader);

mat4 PrepareModelMatrix(vec3 position, vec3 rotation, vec3 scale);
void RenderMesh(Game *game, Mesh *mesh, mat4 model);

#define MESH_H
#endif