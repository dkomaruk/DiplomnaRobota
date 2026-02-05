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

struct SkinnedVertex
{
    union
    {
        Vertex vertex;
        struct
        {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 uv;
        };
    };
    glm::ivec4 boneId;
    glm::vec4 weight;
};

struct VertexText
{
    glm::vec2 position;
    glm::vec2 uv;
};

struct TerrainVertex
{
    glm::vec3 position;
    glm::vec2 uv;
};

struct Mesh
{
    GLuint vao, vbo, ebo;
    int indicesCount, verticesCount;

    GLenum drawMode = GL_TRIANGLES;
};

extern AttribInfo vertexAttribs[3];
extern AttribInfo skinnedVertexAttribs[5];
extern AttribInfo textVertexAttribs[2];
extern AttribInfo terrainVertexAttribs[2];

Mesh CreateVBO(void *vertexData, int vertexDataElements, AttribInfo *attribs,
               int numOfAttribs, GLenum usage = GL_STATIC_DRAW);
void CreateEBO(Mesh *mesh, std::vector<unsigned int> indices);

Mesh CreateMesh(void *verticesData, int verticesCount, int vertexSize = sizeof(Vertex),
                AttribInfo *attribs = vertexAttribs, int numOfAttribs = ArrayCount(vertexAttribs),
                GLenum usage = GL_STATIC_DRAW);
Mesh CreateMesh(void *verticesData, int verticesCount, int vertexSize,
                void *indicesData, int indicesCount,
                AttribInfo *attribs = vertexAttribs,
                int numOfAttribs = ArrayCount(vertexAttribs),
                GLenum usage = GL_STATIC_DRAW);

//void UpdateMesh(Mesh *mesh, std::vector<Vertex> newVertices);
void UpdateMesh(Mesh *mesh, void *newVertices, int size, GLenum usage = GL_DYNAMIC_DRAW);

/**Creates a quad mesh in NDC coordinates from pixel coordinates*/
Mesh CreateQuadNDC(glm::vec2 position, glm::vec2 size);
Mesh CreateUnitQuadStripes();
Mesh *GetUnitQuad();

void RenderMesh(Game *game, Mesh *mesh, glm::mat4 model, GLuint shader, MaterialPhong *material = NULL, GLenum drawMode = GL_TRIANGLES);
void RenderSurface(GLuint shader, Texture *texture, glm::mat4 model);

#define MESH_H
#endif