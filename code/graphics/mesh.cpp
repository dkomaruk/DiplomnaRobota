#include "mesh.h"

#include "texture.h"
#include "game.h"

#include <string>

GLuint LoadTextures(const aiScene *scene, aiMaterial *material, std::string dirPath, std::vector<std::string> *loadedPaths,
                    std::vector<uint32> *loadedTextures, aiTextureType type);
Mesh CreateVBO(std::vector<Vertex> vertices);

GLuint LoadTextures(const aiScene *scene, aiMaterial *material, std::string dirPath, std::vector<std::string> *loadedPaths,
                    std::vector<uint32> *loadedTextures, aiTextureType type)
{
    GLuint result = 0;

    int texturesCount = material->GetTextureCount(type);
    if(!texturesCount) return 0;

    Assert(texturesCount < 2); //TODO: Handle multiple textures of the same type on one mesh

    aiString texPath;
    material->GetTexture(type, 0, &texPath);

    std::string texturePath = "";
    if(texPath.C_Str()[0] == '*')
    {
        texturePath = texPath.C_Str() + 1;
        for(int i = 0; i < loadedPaths->size(); i++)
        {
            if(texturePath == (*loadedPaths)[i]) return (*loadedTextures)[i];
        }

        int index = atoi(texturePath.c_str());
        aiTexture *texture = scene->mTextures[index];

        if(texture->mHeight == 0)
        {
            int w, h;
            uint8 *image = stbi_load_from_memory((uint8 *)texture->pcData, texture->mWidth, &w, &h, 0, 4);
            if(!image) return 0;

            result = CreateGLTexture(image, w, h);
            stbi_image_free(image);
        }
        else
        {
            result = CreateGLTexture((uint8 *)texture->pcData, texture->mWidth, texture->mHeight);
        }
    }
    else
    {
        texturePath = dirPath + '/' + std::string(texPath.C_Str());
        for(int i = 0; i < loadedPaths->size(); i++)
        {
            if(texturePath == (*loadedPaths)[i]) return (*loadedTextures)[i];
        }

        result = CreateTexture((char *)texturePath.c_str());
    }

    loadedPaths->push_back(texturePath);
    loadedTextures->push_back(result);

    return result;
}

void ProcessNode(aiNode *node, int *meshCounter, int *nodeCounter)
{
    if(!node) return;

    *nodeCounter += 1;
    *meshCounter += node->mNumMeshes;

    for(uint32 i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], meshCounter, nodeCounter);
    }
}

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

    int meshCounter = 0;
    int nodeCounter = 0;
    ProcessNode(scene->mRootNode, &meshCounter, &nodeCounter);
    SDL_Log("mNumMeshes: %d, Mesh counter: %d, Node counter: %d", scene->mNumMeshes, meshCounter, nodeCounter);

    std::string dirPath = filepath;
    size_t found = dirPath.find_last_of("\\/");
    dirPath = (found == std::string::npos) ? dirPath : dirPath.substr(0, found);

    result.meshes = (Mesh *)malloc(sizeof(Mesh) * scene->mNumMeshes);
    result.numOfMeshes = scene->mNumMeshes;

    std::vector<std::string> loadedDiffusePaths, loadedSpecularPaths;
    std::vector<uint32> diffuseTextures, specularTextures;

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

        GLuint diffuseTexture = LoadTextures(scene, material, dirPath, &loadedDiffusePaths, &diffuseTextures, aiTextureType_DIFFUSE);
        GLuint specularTexture = LoadTextures(scene, material, dirPath, &loadedSpecularPaths, &specularTextures, aiTextureType_SPECULAR);

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
    GLuint shader = game->outlinePass ? game->outlineShader : mesh->shader;
    //GLuint shader = game->outlineShader;
    UseShader(shader);

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(glGetUniformLocation(shader, "u_normalMatrix"), 1, GL_FALSE, value_ptr(normalMatrix));

    ShaderSetMatrix4(shader, "u_model", model);

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