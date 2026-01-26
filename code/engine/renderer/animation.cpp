#include "animation.h"

Mesh CreateMesh(std::vector<SkinnedVertex> vertices)
{
    AttribInfo skinnedVertexAttribs[5];

    skinnedVertexAttribs[0] = {0, 4, GL_INT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, boneId)};
    skinnedVertexAttribs[1] = {1, 4, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, weight)};
    skinnedVertexAttribs[2] = {2, 3, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, normal)};
    skinnedVertexAttribs[3] = {3, 3, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, position)};
    skinnedVertexAttribs[4] = {4, 2, GL_FLOAT, sizeof(SkinnedVertex), (void *)offsetof(SkinnedVertex, uv)};

    uint32 verticesElements = (uint32)vertices.size() * 16;
    return CreateVBO(vertices.data(), verticesElements, skinnedVertexAttribs, 5);
}

Mesh CreateMesh(std::vector<SkinnedVertex> vertices, std::vector<unsigned int> indices)
{
    Mesh mesh = CreateMesh(vertices);
    CreateEBO(&mesh, indices);
    return mesh;
}

glm::mat4 AssimpMat4ToGLM(aiMatrix4x4 m)
{
    glm::mat4 result(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    );

    return result;
}