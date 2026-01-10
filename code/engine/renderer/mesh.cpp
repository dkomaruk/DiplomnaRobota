#include "mesh.h"

#include "texture.h"
#include "shader.h"
#include "game.h"

#include <stb_image.h>

#include <glm/gtc/type_ptr.hpp>

#include <string>

Texture LoadTextures(const aiScene *scene, aiMaterial *material,
                    std::string dirPath, std::vector<std::string> *loadedPaths,
                    std::vector<Texture> *loadedTextures, aiTextureType type)
{
    Texture result = {};

    int texturesCount = material->GetTextureCount(type);
    if(!texturesCount) return result; //Return a missing texture placeholder

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
            if(!image) return result; //TODO: Handle multiple textures of the same type on one mesh

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

Model *ImportModel(char *filepath, GLuint shader, uint32 flags)
{
    Model *result = (Model *)malloc(sizeof(Model));
    result->numOfMeshes = -1;

    const aiScene *scene = aiImportFile(filepath, flags);
    if(!scene)
    {
        SDL_Log("Failed to load %s. Error: %s", filepath, aiGetErrorString());
        return result;
    }

    std::string dirPath = filepath;
    size_t found = dirPath.find_last_of("\\/");
    dirPath = (found == std::string::npos) ? dirPath : dirPath.substr(0, found);

    result->mesh = (Mesh *)malloc(sizeof(Mesh) * scene->mNumMeshes);
    result->material = (MaterialPhong *)malloc(sizeof(MaterialPhong) * scene->mNumMeshes);
    result->numOfMeshes = scene->mNumMeshes;

    std::vector<std::string> loadedDiffusePaths, loadedSpecularPaths;
    std::vector<Texture> diffuseTextures, specularTextures;

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
            vertex.position = glm::vec3(pos.x, pos.y, pos.z);
            vertex.normal = glm::vec3(norm.x, norm.y, norm.z);
            if(hasUVs)
            {
                aiVector3D uv = mesh->mTextureCoords[0][j];
                vertex.uv = glm::vec2(uv.x, uv.y);
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

        Texture diffuseTexture = LoadTextures(scene, material, dirPath, &loadedDiffusePaths, &diffuseTextures, aiTextureType_DIFFUSE);
        Texture specularTexture = LoadTextures(scene, material, dirPath, &loadedSpecularPaths, &specularTextures, aiTextureType_SPECULAR);

        result->mesh[i] = CreateMesh(vertices, indices);

        MaterialPhong phongMaterial = {shader, diffuseTexture, specularTexture, {}, 32.0f};
        result->material[i] = phongMaterial;
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

Mesh CreateMesh(std::vector<Vertex> vertices)
{
    Mesh mesh = CreateVBO(vertices);
    return mesh;
}

Mesh CreateTextVBO(std::vector<VertexText> vertices, GLenum usage = GL_DYNAMIC_DRAW)
{
    Mesh mesh = {};

    mesh.verticesCount = (uint32)vertices.size();

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexText), &vertices[0], usage);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexText), (void *)offsetof(VertexText, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexText), (void *)offsetof(VertexText, uv));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mesh;
}

Mesh CreateTextMesh(std::vector<VertexText> vertices)
{
    Mesh mesh = CreateTextVBO(vertices);
    return mesh;
}

Mesh CreateMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
    Mesh mesh = CreateMesh(vertices);
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

Mesh CreateQuadNDC(glm::vec2 position, glm::vec2 size)
{
    glm::vec2 window = glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);

    //Position and size in NDC coordinates
    glm::vec3 p = glm::vec3(glm::vec2((position.x / window.x) * 2.0f - 1.0f, 1.0f - (position.y / window.y) * 2.0f), 0.0f);
    glm::vec3 s = glm::vec3((size / window) * 2.0f, 0.0f);

    //Normals
    glm::vec3 n = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v1 = {p, n, glm::vec2(0.0f, 1.0f)};
    Vertex v2 = {glm::vec3(p.x, p.y - s.y, p.z), n, glm::vec2(0.0f, 0.0f)};
    Vertex v3 = {glm::vec3(p.x + s.x, p.y - s.y, p.z), n, glm::vec2(1.0f, 0.0f)};
    Vertex v4 = {glm::vec3(p.x + s.x, p.y, p.z), n, glm::vec2(1.0f, 1.0f)};

    std::vector<Vertex> vertices = {v1, v2, v3, v1, v3, v4};

    Mesh quad = CreateMesh(vertices);

    return quad;
}

Mesh CreateUnitQuad()
{
    glm::vec3 n = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v1 = {glm::vec3(0.0f, 1.0f, 0.0f), n, glm::vec2(0.0f, 0.0f)};
    Vertex v2 = {glm::vec3(0.0f, 0.0f, 0.0f), n, glm::vec2(0.0f, 1.0f)};
    Vertex v3 = {glm::vec3(1.0f, 0.0f, 0.0f), n, glm::vec2(1.0f, 1.0f)};
    Vertex v4 = {glm::vec3(1.0f, 1.0f, 0.0f), n, glm::vec2(1.0f, 0.0f)};
    std::vector<Vertex> vertices = {v1, v2, v3, v1, v3, v4};

    return CreateMesh(vertices);
}

Mesh CreateUnitQuadStripes()
{
    glm::vec3 n = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v1 = {glm::vec3(0.0f, 1.0f, 0.0f), n, glm::vec2(0.0f, 0.0f)};
    Vertex v2 = {glm::vec3(0.0f, 0.0f, 0.0f), n, glm::vec2(0.0f, 1.0f)};
    Vertex v3 = {glm::vec3(1.0f, 0.0f, 0.0f), n, glm::vec2(1.0f, 1.0f)};
    Vertex v4 = {glm::vec3(1.0f, 1.0f, 0.0f), n, glm::vec2(1.0f, 0.0f)};
    std::vector<Vertex> vertices = {v2, v3, v1, v4};

    return CreateMesh(vertices);
}

Mesh *GetUnitQuad()
{
    static Mesh unitQuad = CreateUnitQuad();
    return &unitQuad;
}

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 _scale)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    //TODO: Rotation using quaternions
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, _scale);
    return model;
}

void RenderMesh(Game *game, Mesh *mesh, MaterialPhong *material, glm::mat4 model)
{
    GLuint shader = material->shader;
    if(game->outlinePass)
    {
        shader = game->outlineShader;
    }
    if(game->pickingPass)
    {
        shader = game->pickingShader;
    }

    UseShader(shader);

    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    glUniformMatrix3fv(glGetUniformLocation(shader, "u_normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    ShaderSetMatrix4(shader, "u_model", model);

    ShaderSetMaterial(shader, material);
    SetTexture(&material->diffuseTexture, 0);
    SetTexture(&material->specularTexture, 1);

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

void RenderSurface(GLuint shader, Texture *texture, glm::mat4 model)
{
    Mesh *quad = GetUnitQuad();

    ShaderSetMatrix4(shader, "u_model", model);

    glBindVertexArray(quad->vao);
    glDrawArrays(GL_TRIANGLES, 0, quad->verticesCount);

    glBindVertexArray(0);
    //glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderModel(Game *game, Model *model, glm::mat4 modelMat)
{
    for(int i = 0; i < model->numOfMeshes; i++)
    {
        RenderMesh(game, &model->mesh[i], &model->material[i], modelMat);
    }
}