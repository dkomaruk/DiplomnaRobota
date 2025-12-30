#ifndef MESH_H

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

struct Game;

struct MaterialPhong
{
    GLuint shader;

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

struct VertexText
{
    vec2 position;
    vec2 uv;
};

struct Mesh
{
    GLuint vao, vbo;
    int indicesCount, verticesCount;
};

struct Model
{
    Mesh *mesh;
    MaterialPhong *material;
    int numOfMeshes;
};

Mesh CreateMesh(std::vector<Vertex> vertices);
Mesh CreateTextMesh(std::vector<VertexText> vertices);

Mesh CreateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

/**Creates a quad mesh in NDC coordinates from pixel coordinates*/
Mesh CreateQuadNDC(vec2 position, vec2 size);
Mesh GetUnitQuad();

Model *ImportModel(char *filepath, GLuint shader, uint32 flags = 0);

mat4 PrepareModelMatrix(vec3 position, vec3 rotation, vec3 scale);
void RenderMesh(Game *game, Mesh *mesh, MaterialPhong *material, mat4 model);
void RenderModel(Game *game, Model *model, mat4 modelMat);

#define MESH_H
#endif