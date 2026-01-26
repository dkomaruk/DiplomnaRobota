#ifndef MESH_H

#include "texture.h"

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

    Texture diffuseTexture; //GL_TEXTURE0
    Texture specularTexture; //GL_TEXTURE1
    Texture emissionTexture; //GL_TEXTURE2

    float shininess;
};

struct AttribInfo
{
    GLuint attribLocation;
    GLint size;
    GLenum type;
    GLsizei stride;
    void *pointer;
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
    GLuint vao, vbo, ebo;
    int indicesCount, verticesCount;

    GLenum drawMode = GL_TRIANGLES;
};

struct Model
{
    Mesh *mesh;
    MaterialPhong *material;
    int numOfMeshes;
};

Mesh CreateVBO(void *vertexData, int vertexDataElements, AttribInfo *attribs, int numOfAttribs);
void CreateEBO(Mesh *mesh, std::vector<unsigned int> indices);

Mesh CreateMesh(std::vector<Vertex> vertices);
Mesh CreateMesh(std::vector<float> vertices, AttribInfo *attribs = 0, int numOfAttribs = 0);
Mesh CreateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
Mesh CreateMesh(std::vector<float> vertices, std::vector<unsigned int> indices,
                AttribInfo *attribs = 0, int numOfAttribs = 0);

Mesh CreateTextMesh(std::vector<VertexText> vertices);

void UpdateMesh(Mesh *mesh, std::vector<Vertex> newVertices);

/**Creates a quad mesh in NDC coordinates from pixel coordinates*/
Mesh CreateQuadNDC(glm::vec2 position, glm::vec2 size);
Mesh CreateUnitQuadStripes();
Mesh *GetUnitQuad();

Model *ImportModel(char *filepath, GLuint shader, uint32 flags = 0);

void RenderMesh(Game *game, Mesh *mesh, MaterialPhong *material, glm::mat4 model, GLenum drawMode = GL_TRIANGLES);
void RenderSurface(GLuint shader, Texture *texture, glm::mat4 model);

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
void RenderModel(Game *game, Model *model, glm::mat4 modelMat);

#define MESH_H
#endif