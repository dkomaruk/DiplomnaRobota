#include "mesh.h"

#include "texture.h"
#include "game.h"

#include <string>

Model ImportModel(char *filepath, GLuint shader, uint32 flags)
{
    Model result = {};
    result.numOfMeshes = -1;

    const aiScene *scene = aiImportFile(filepath, flags);
    if(!scene)
    {
        SDL_Log("Failed to load %s. Error: %s", filepath, aiGetErrorString());
        return result;
    }

    std::string dirPath = filepath;
    size_t found = dirPath.find_last_of("\\/");
    dirPath = (found == std::string::npos) ? dirPath : dirPath.substr(0, found);

    result.meshes = (Mesh *)malloc(sizeof(Mesh) * scene->mNumMeshes);
    result.numOfMeshes = scene->mNumMeshes;

    std::vector<std::string> loadedTexturePaths;
    std::vector<uint32> diffuseTextures;

    for(uint32 i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[i];
        bool hasUVs = mesh->HasTextureCoords(0);

        std::vector<Vertex> vertices;
        std::vector<uint32> indices;
        for(uint32 j = 0; j < mesh->mNumVertices; j++)
        {
            aiVector3D pos = mesh->mVertices[j];
            aiVector3D norm = mesh->mNormals[j];

            Vertex vertex = {};
            vertex.position = vec3(pos.x, pos.y, pos.z);
            vertex.normal = vec3(norm.x, norm.y, norm.z);
            if(hasUVs)
            {
                aiVector3D uv = mesh->mTextureCoords[0][j];
                vertex.uv = vec2(uv.x, uv.y);
            }

            vertices.push_back(vertex);
        }

        for(uint32 j = 0; j < mesh->mNumFaces; j++)
        {
            for(uint32 k = 0; k < mesh->mFaces[j].mNumIndices; k++)
            {
                indices.push_back(mesh->mFaces[j].mIndices[k]);
            }
        }

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        GLuint diffuseTexture = 0;
        GLuint specularTexture = 0;

        for(uint32 j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); j++)
        {
            aiString texPath;
            material->GetTexture(aiTextureType_DIFFUSE, j, &texPath);

            std::string texturePath = dirPath + '/' + std::string(texPath.C_Str());
            bool alreadyLoaded = false;
            for(int k = 0; k < loadedTexturePaths.size(); k++)
            {
                if(texturePath == loadedTexturePaths[k])
                {
                    alreadyLoaded = true;
                    diffuseTexture = diffuseTextures[k];
                    break;
                }
            }

            if(!alreadyLoaded)
            {
                loadedTexturePaths.push_back(texturePath);
                diffuseTexture = CreateTexture((char *)texturePath.c_str(), 0);
                diffuseTextures.push_back(diffuseTexture);
            }
        }

        result.meshes[i] = CreateMesh(vertices, indices, shader);
        result.meshes[i].material.diffuseTexture = diffuseTexture;
        result.meshes[i].material.specularTexture = specularTexture;
    }

    return result;
}

Mesh CreateVBO(std::vector<Vertex> vertices)
{
    Mesh mesh = {};

    mesh.verticesCount = (uint32)vertices.size();

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mesh;
}

Mesh CreateMesh(std::vector<Vertex> vertices, GLuint shader)
{
    Mesh mesh = CreateVBO(vertices);
    mesh.shader = shader;

    return mesh;
}

Mesh CreateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, GLuint shader)
{
    Mesh mesh = CreateMesh(vertices, shader);
    mesh.indicesCount = (uint32)indices.size();

    glBindVertexArray(mesh.vao);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return mesh;
}

mat4 PrepareModelMatrix(vec3 position, vec3 rotation, vec3 _scale)
{
    mat4 model = mat4(1.0f);
    model = translate(model, position);

    //TODO: Rotation using quaternions
    model = rotate(model, radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
    model = rotate(model, radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
    model = rotate(model, radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));

    model = scale(model, _scale);
    return model;
}

void RenderMesh(Game *game, Mesh *mesh, mat4 model)
{
    GLuint shader = mesh->shader;
    UseShader(shader);

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(glGetUniformLocation(shader, "u_normalMatrix"), 1, GL_FALSE, value_ptr(normalMatrix));

    ShaderSetMatrix4(shader, "u_model", model);
    ShaderSetMatrix4(shader, "u_view", game->view);
    ShaderSetMatrix4(shader, "u_projection", game->projection);

    ShaderSetMaterial(shader, &mesh->material);
    SetTexture(mesh->material.diffuseTexture, 0);
    SetTexture(mesh->material.specularTexture, 1);

    glBindVertexArray(mesh->vao);
    if(mesh->indicesCount > 0)
    {
        glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mesh->verticesCount);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderModel(Game *game, Model *model, mat4 modelMat)
{
    for(int i = 0; i < model->numOfMeshes; i++)
    {
        RenderMesh(game, &model->meshes[i], modelMat);
    }
}