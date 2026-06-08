#include "mesh.h"

#include "texture.h"
#include "shader.h"
#include "game.h"

#include <glm/gtc/type_ptr.hpp>

#include <string>

AttribInfo vertexAttribs[3] = {
    {0, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, position)},
    {1, 3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, normal)},
    {2, 2, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, uv)}
};

AttribInfo skinnedVertexAttribs[5] = {
    {0, 3, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, position)},
    {1, 3, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, normal)},
    {2, 2, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, uv)},
    {3, 4, GL_INT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, boneId)},
    {4, 4, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, weight)}
};

AttribInfo textVertexAttribs[2] = {
    {0, 2, GL_FLOAT, sizeof(VertexText), (void *)offsetof(VertexText, position)},
    {1, 2, GL_FLOAT, sizeof(VertexText), (void *)offsetof(VertexText, uv)}
};

//AttribInfo terrainVertexAttribs[3] = {
AttribInfo terrainVertexAttribs[2] = {
    {0, 3, GL_FLOAT, sizeof(TerrainVertex), (void *)0},
    {1, 2, GL_FLOAT, sizeof(TerrainVertex), (void *)(sizeof(float) *3)}
    //{2, 3, GL_FLOAT, sizeof(TerrainVertex), (void *)(sizeof(float) *5)}
};

Mesh CreateVBO(void *verticesData, int verticesCount, int vertexSize, AttribInfo *attribs, int numOfAttribs, GLenum usage)
{
    Mesh mesh = {};
    mesh.verticesCount = verticesCount;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, verticesCount * vertexSize, verticesData, usage);

    for(int i = 0; i < numOfAttribs; ++i)
    {
        glEnableVertexAttribArray(attribs[i].attribLocation);
        switch(attribs[i].type)
        {
            case GL_FLOAT:
            {
                glVertexAttribPointer(attribs[i].attribLocation, attribs[i].size, attribs[i].type,
                                     GL_FALSE, attribs[i].stride, attribs[i].pointer);
            } break;

            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_UNSIGNED_BYTE:
            {
                glVertexAttribIPointer(attribs[i].attribLocation, attribs[i].size, attribs[i].type,
                                       attribs[i].stride, attribs[i].pointer);
            } break;

            CaseNotImplemented
        }
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mesh;
}

void CreateEBO(void *indicesData, int indicesCount, Mesh *mesh)
{
    mesh->indicesCount = indicesCount;

    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned int), indicesData, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Mesh CreateMesh(void *verticesData, int verticesCount, int vertexSize,
                AttribInfo *attribs, int numOfAttribs, GLenum usage)
{
    Mesh mesh = {};
    if(attribs && numOfAttribs > 0)
    {
        mesh = CreateVBO(verticesData, verticesCount, vertexSize, attribs, numOfAttribs, usage);
    }

    return mesh;
}

Mesh CreateMesh(void *verticesData, int verticesCount, int vertexSize, void *indicesData,
                int indicesCount, AttribInfo *attribs, int numOfAttribs, GLenum usage)
{
    Mesh mesh = CreateMesh(verticesData, verticesCount, vertexSize, attribs, numOfAttribs, usage);
    CreateEBO(indicesData, indicesCount, &mesh);
    return mesh;
}

void DeleteMesh(Mesh *mesh)
{
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
}

void UpdateMesh(Mesh *mesh, void *newVertices, int size, GLenum usage)
{
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, size, newVertices, usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh CreateQuadNDC(glm::vec2 position, glm::vec2 quadSize, glm::vec2 viewportSize)
{
    glm::vec3 p = glm::vec3(glm::vec2((position.x / viewportSize.x) * 2.0f - 1.0f, 1.0f - (position.y / viewportSize.y) * 2.0f), 0.0f);
    glm::vec3 s = glm::vec3((quadSize / viewportSize) * 2.0f, 0.0f);

    glm::vec3 n = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v1 = {p, n, glm::vec2(0.0f, 1.0f)};
    Vertex v2 = {glm::vec3(p.x, p.y - s.y, p.z), n, glm::vec2(0.0f, 0.0f)};
    Vertex v3 = {glm::vec3(p.x + s.x, p.y - s.y, p.z), n, glm::vec2(1.0f, 0.0f)};
    Vertex v4 = {glm::vec3(p.x + s.x, p.y, p.z), n, glm::vec2(1.0f, 1.0f)};

    std::vector<Vertex> vertices = {v1, v2, v3, v1, v3, v4};

    Mesh quad = CreateMesh(&vertices[0], vertices.size());
    return quad;
}

Mesh CreateUnitQuad()
{
    glm::vec3 n = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v1 = {glm::vec3(-0.5f, 0.5f, 0.0f), n, glm::vec2(0.0f, 0.0f)};
    Vertex v2 = {glm::vec3(-0.5f, -0.5f, 0.0f), n, glm::vec2(0.0f, 1.0f)};
    Vertex v3 = {glm::vec3(0.5f, -0.5f, 0.0f), n, glm::vec2(1.0f, 1.0f)};
    Vertex v4 = {glm::vec3(0.5f, 0.5f, 0.0f), n, glm::vec2(1.0f, 0.0f)};
    std::vector<Vertex> vertices = {v1, v2, v3, v1, v3, v4};

    return CreateMesh(&vertices[0], vertices.size());
}

Mesh *GetUnitQuad()
{
    static Mesh unitQuad = CreateUnitQuad();
    return &unitQuad;
}

Mesh CreateUnitQuadStripes()
{
    glm::vec3 n = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v1 = {glm::vec3(-0.5f, 1.0f, 0.0f), n, glm::vec2(0.0f, 1.0f)};
    Vertex v2 = {glm::vec3(-0.5f, 0.0f, 0.0f), n, glm::vec2(0.0f, 0.0f)};
    Vertex v3 = {glm::vec3(0.5f, 0.0f, 0.0f), n, glm::vec2(1.0f, 0.0f)};
    Vertex v4 = {glm::vec3(0.5f, 1.0f, 0.0f), n, glm::vec2(1.0f, 1.0f)};
    std::vector<Vertex> vertices = {v2, v3, v1, v4};

    return CreateMesh(&vertices[0], vertices.size(), sizeof(Vertex), vertexAttribs, ArrayCount(vertexAttribs));
}

void RenderMesh(Game *game, Mesh *mesh, glm::mat4 model, GLuint shader, MaterialPhong *material, GLenum drawMode)
{
    UseShader(shader);

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    glUniformMatrix3fv(glGetUniformLocation(shader, "u_normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    ShaderSetMatrix4(shader, "u_model", model);

    if(material)
    {
        ShaderSetMaterial(shader, material);
        SetTexture(&material->diffuseTexture, 0);
        SetTexture(&material->specularTexture, 1);
    }

    SetTexture(game->shadowMapFbo.depth.id, 3);
    ShaderSetInt(shader, "u_shadowMap", 3);

    glBindVertexArray(mesh->vao);
    if(mesh->indicesCount > 0)
    {
        glDrawElements(drawMode, mesh->indicesCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(drawMode, 0, mesh->verticesCount);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSurface(GLuint shader, Texture *texture, glm::mat4 model)
{
    Mesh *quad = GetUnitQuad();

    ShaderSetMatrix4(shader, "u_model", model);

    glBindVertexArray(quad->vao);
    glDrawArrays(GL_TRIANGLES, 0, quad->verticesCount);

    glBindVertexArray(0);
    //glBindTexture(GL_TEXTURE_2D, 0);
}