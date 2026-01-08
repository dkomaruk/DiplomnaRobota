#ifndef MESH_H

#include "defines.h"

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
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct VertexText
{
    glm::vec2 position;
    glm::vec2 uv;
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
Mesh CreateQuadNDC(glm::vec2 position, glm::vec2 size);
Mesh GetUnitQuad();

Model *ImportModel(char *filepath, GLuint shader, uint32 flags = 0);

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
void RenderMesh(Game *game, Mesh *mesh, MaterialPhong *material, glm::mat4 model);
void RenderModel(Game *game, Model *model, glm::mat4 modelMat);

#define MESH_H
#endif