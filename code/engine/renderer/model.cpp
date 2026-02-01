#include "model.h"

#include "mesh.h"
#include "texture.h"

#include <SDL3/SDL.h>

#include <stb_image.h>

#include <string>
#include <vector>

Texture ImportTextures(const aiScene *scene, aiMaterial *material,
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
            if(!image) return result;

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

void LoadVertexData(aiMesh *mesh, int vertexId, Vertex *vertex)
{
    aiVector3D pos = mesh->mVertices[vertexId];
    aiVector3D norm = mesh->mNormals[vertexId];

    vertex->position = glm::vec3(pos.x, pos.y, pos.z);
    vertex->normal = glm::vec3(norm.x, norm.y, norm.z);
    if(mesh->HasTextureCoords(0))
    {
        aiVector3D uv = mesh->mTextureCoords[0][vertexId];
        vertex->uv = glm::vec2(uv.x, uv.y);
    }
}

std::vector<SkinnedVertex> LoadAnimatedVerticesData(aiMesh *mesh, Skeleton *skeleton)
{
    std::vector<SkinnedVertex> vertices;
    vertices.resize(mesh->mNumVertices);

    //Load vertices
    for(uint32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        vertices[vertexIndex].boneId = glm::ivec4(-1);
        LoadVertexData(mesh, vertexIndex, &vertices[vertexIndex].vertex);
    }

    //Load bones
    for(uint32 boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
    {
        aiBone *aBone = mesh->mBones[boneIndex];

        int boneId = -1;
        std::string boneName = aBone->mName.C_Str();

        if(!skeleton->boneMap.count(boneName))
        {
            Bone bone = {};
            bone.invBindPose = AssimpMat4ToGLM(aBone->mOffsetMatrix);
            bone.name = boneName;
            bone.id = skeleton->numOfBones;
            boneId = bone.id;

            skeleton->boneMap[bone.name] = bone;
            skeleton->numOfBones++;
        }
        else
        {
            boneId = skeleton->boneMap[aBone->mName.C_Str()].id;
        }

        Assert(boneId != -1)

        //Load bone weights into vertices
        for(uint32 weightIndex = 0; weightIndex < aBone->mNumWeights; weightIndex++)
        {
            int vertexId = aBone->mWeights[weightIndex].mVertexId;
            Assert((uint32)vertexId < mesh->mNumVertices)
            for(int i = 0; i < 4; i++)
            {
                if(vertices[vertexId].boneId[i] < 0)
                {
                    vertices[vertexId].weight[i] = aBone->mWeights[weightIndex].mWeight;
                    vertices[vertexId].boneId[i] = boneId;
                    break;
                }
            }
        }
    }

    return vertices;
}

std::vector<Vertex> LoadStaticVerticesData(aiMesh *mesh)
{
    std::vector<Vertex> vertices;
    vertices.resize(mesh->mNumVertices);

    for(uint32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        LoadVertexData(mesh, vertexIndex, &vertices[vertexIndex]);
    }
    return vertices;
}

Model *ImportModel(char *filepath, GLuint shader, uint32 flags, uint16 type, float scale)
{
    Model *result = (Model *)calloc(1, sizeof(Model));
    result->numOfMeshes = -1;

    aiSetImportPropertyFloat(aiCreatePropertyStore(), AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale);
    const aiScene *scene = aiImportFile(filepath, flags);
    if(!scene)
    {
        SDL_Log("Failed to load %s. Error: %s", filepath, aiGetErrorString());
        return result;
    }

    std::string dirPath = filepath;
    size_t found = dirPath.find_last_of("\\/");
    dirPath = (found == std::string::npos) ? dirPath : dirPath.substr(0, found);

    result->numOfMeshes = scene->mNumMeshes;
    result->mesh = (Mesh *)calloc(result->numOfMeshes, sizeof(Mesh));
    result->material = (MaterialPhong *)calloc(scene->mNumMeshes, sizeof(MaterialPhong));

    if(type == ModelType_Animated)
    {
        result->animData.skeleton = new Skeleton{};

        result->animData.numOfAnimations = scene->mNumAnimations;
        result->animData.animations = new Animation[result->animData.numOfAnimations]{};

        result->animData.numOfMatrices = 100;
        result->animData.skinningMatrices = (glm::mat4 *)calloc(result->animData.numOfMatrices, sizeof(glm::mat4));

        result->animData.scene = (aiScene *)scene;
    }

    std::vector<std::string> loadedDiffusePaths, loadedSpecularPaths;
    std::vector<Texture> diffuseTextures, specularTextures;

    for(uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];
        bool hasUVs = mesh->HasTextureCoords(0);

        std::vector<uint32> indices;
        for(uint32 j = 0; j < mesh->mNumFaces; j++)
        {
            for(uint32 k = 0; k < mesh->mFaces[j].mNumIndices; k++)
            {
                indices.push_back(mesh->mFaces[j].mIndices[k]);
            }
        }

        if(type == ModelType_Static)
        {
            std::vector<Vertex> vertices = LoadStaticVerticesData(mesh);
            result->mesh[meshIndex] = CreateMesh(&vertices[0], vertices.size(), sizeof(Vertex),
                                                 &indices[0], indices.size());
        }
        else if(type == ModelType_Animated)
        {
            std::vector<SkinnedVertex> vertices = LoadAnimatedVerticesData(mesh, result->animData.skeleton);
            result->mesh[meshIndex] = CreateMesh(&vertices[0], vertices.size(), sizeof(SkinnedVertex), &indices[0],
                                                 indices.size(), skinnedVertexAttribs, ArrayCount(skinnedVertexAttribs));
        }

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        Texture diffuseTexture = ImportTextures(scene, material, dirPath, &loadedDiffusePaths, &diffuseTextures, aiTextureType_DIFFUSE);
        Texture specularTexture = ImportTextures(scene, material, dirPath, &loadedSpecularPaths, &specularTextures, aiTextureType_SPECULAR);

        MaterialPhong phongMaterial = {shader, diffuseTexture, specularTexture, {}, 32.0f};
        result->material[meshIndex] = phongMaterial;
    }

    if(type == ModelType_Animated)
    {
        AnimatedModel *animData = &result->animData;
        for(int i = 0; i < animData->numOfMatrices; i++)
        {
            animData->skinningMatrices[i] = glm::mat4(1.0f);
        }

        //int numOfNodes = 0;
        //CountNodes(scene->mRootNode, &numOfNodes);
        //skeleton.nodes = (Node *)calloc(numOfNodes, sizeof(Node));

        for(uint32 animationIndex = 0; animationIndex < scene->mNumAnimations; animationIndex++)
        {
            Animation *animation = &animData->animations[animationIndex];
            aiAnimation *anim = scene->mAnimations[animationIndex];

            animation->ticksPerSecond = (float)anim->mTicksPerSecond;
            animation->numOfFrames = (int)anim->mDuration;

            for(uint32 sampleIndex = 0; sampleIndex < anim->mNumChannels; sampleIndex++)
            {
                aiNodeAnim *channel = anim->mChannels[sampleIndex];
                std::string nodeName = channel->mNodeName.C_Str();

                AnimationSample sample = {};

                sample.numOfPositions = channel->mNumPositionKeys;
                sample.posKeys = (KeyPosition *)calloc(channel->mNumPositionKeys, sizeof(KeyPosition));
                for(uint32 posKeyIndex = 0; posKeyIndex < channel->mNumPositionKeys; posKeyIndex++)
                {
                    aiVectorKey *pk = &channel->mPositionKeys[posKeyIndex];
                    sample.posKeys[posKeyIndex].time = (float)pk->mTime;
                    sample.posKeys[posKeyIndex].position = glm::vec3(pk->mValue.x, pk->mValue.y, pk->mValue.z);
                }

                sample.numOfRotations = channel->mNumRotationKeys;
                sample.rotKeys = (KeyRotation *)calloc(channel->mNumRotationKeys, sizeof(KeyRotation));
                for(uint32 rotKeyIndex = 0; rotKeyIndex < channel->mNumRotationKeys; rotKeyIndex++)
                {
                    aiQuatKey *rk = &channel->mRotationKeys[rotKeyIndex];
                    sample.rotKeys[rotKeyIndex].time = (float)rk->mTime;
                    sample.rotKeys[rotKeyIndex].rotation = glm::quat(rk->mValue.w, rk->mValue.x,
                                                                    rk->mValue.y, rk->mValue.z);
                }

                sample.numOfScalings = channel->mNumScalingKeys;
                sample.scaleKeys = (KeyScale *)calloc(channel->mNumScalingKeys, sizeof(KeyScale));
                for(uint32 scaleKeyIndex = 0; scaleKeyIndex < channel->mNumScalingKeys; scaleKeyIndex++)
                {
                    aiVectorKey *sk = &channel->mScalingKeys[scaleKeyIndex];
                    sample.scaleKeys[scaleKeyIndex].time = (float)sk->mTime;
                    sample.scaleKeys[scaleKeyIndex].scale = glm::vec3(sk->mValue.x, sk->mValue.y, sk->mValue.z);
                }

                animation->samples[nodeName] = sample;
            }
        }
    }

    return result;
}

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    //TODO: Rotation using quaternions
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, scale);
    return model;
}

void RenderModel(Game *game, Model *model, glm::mat4 modelMat)
{
    for(int i = 0; i < model->numOfMeshes; i++)
    {
        RenderMesh(game, &model->mesh[i], &model->material[i], modelMat);
    }
}