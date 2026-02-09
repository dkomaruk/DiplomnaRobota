#include "model.h"

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "game.h"

#include <SDL3/SDL.h>

#include <glm/gtc/type_ptr.hpp>

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

std::vector<SkinnedVertex> LoadAnimatedVerticesData(aiMesh *mesh, Skeleton *skeleton,
                                                    std::unordered_map<std::string, int> &boneMap)
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

        std::string boneName = aBone->mName.C_Str();
        int boneId = boneMap[boneName];

        skeleton->invBindPoses[boneId] = AssimpMat4ToGLM(aBone->mOffsetMatrix);

        Assert(boneId >= 0 && boneId < skeleton->numOfBones)

        //Load bone weights into vertices
        for(uint32 weightIndex = 0; weightIndex < aBone->mNumWeights; weightIndex++)
        {
            int vertexId = aBone->mWeights[weightIndex].mVertexId;
            Assert((uint32)vertexId < mesh->mNumVertices)

            //The bone with the largest influence on this vertex gets to extend its AABB with vertex position
            float weight = aBone->mWeights[weightIndex].mWeight;
            if(weight > 0.5f)
            {
                glm::vec3 vertexInBoneSpace = AssimpVec3ToGLM(aBone->mOffsetMatrix * mesh->mVertices[vertexId]);
                ExpandAABB(&skeleton->boneAABBs[boneId], vertexInBoneSpace);
            }

            for(int i = 0; i < 4; i++)
            {
                if(vertices[vertexId].boneId[i] < 0)
                {
                    vertices[vertexId].weight[i] = weight;
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

void CountNodes(aiNode *node, int *counter, std::unordered_map<std::string, int> &nameToNodeIndex)
{
    if(!node) return;

    nameToNodeIndex[node->mName.C_Str()] = *counter;
    *counter += 1;

    for(uint32 childIndex = 0; childIndex < node->mNumChildren; childIndex++)
    {
        CountNodes(node->mChildren[childIndex], counter, nameToNodeIndex);
    }
}

void MergeAABB(AABB *dest, AABB *src)
{
    dest->min = glm::min(dest->min, src->min);
    dest->max = glm::max(dest->max, src->max);
}

void ExpandAABB(AABB *aabb, glm::vec3 point)
{
    aabb->min = glm::min(aabb->min, point);
    aabb->max = glm::max(aabb->max, point);
}

void UpdateAABBCorners(AABB *aabb)
{
    aabb->corners[0] = {aabb->min.x, aabb->min.y, aabb->min.z};
    aabb->corners[1] = {aabb->max.x, aabb->min.y, aabb->min.z};
    aabb->corners[2] = {aabb->min.x, aabb->max.y, aabb->min.z};
    aabb->corners[3] = {aabb->max.x, aabb->max.y, aabb->min.z};
    aabb->corners[4] = {aabb->min.x, aabb->min.y, aabb->max.z};
    aabb->corners[5] = {aabb->max.x, aabb->min.y, aabb->max.z};
    aabb->corners[6] = {aabb->min.x, aabb->max.y, aabb->max.z};
    aabb->corners[7] = {aabb->max.x, aabb->max.y, aabb->max.z};
}

void UpdateAABBMesh(AABB *aabb, Mesh *aabbMesh, bool recalculateCorners)
{
    if(recalculateCorners)
    {
        UpdateAABBCorners(aabb);
    }

    glBindBuffer(GL_ARRAY_BUFFER, aabbMesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(aabb->corners), aabb->corners);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

AABB TransformAABB(AABB *aabb, glm::mat4 transform)
{
    UpdateAABBCorners(aabb);

    AABB result = {glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX)};
    for(int cornerIndex = 0; cornerIndex < ArrayCount(aabb->corners); cornerIndex++)
    {
        ExpandAABB(&result, glm::vec3(transform * glm::vec4(aabb->corners[cornerIndex], 1.0f)));
    }

    return result;
}

void FlattenAssimpHierarchy(aiScene *scene, aiNode *aNode, Model *model,
                            int parentId, glm::mat4 parentTransform,
                            std::unordered_map<std::string, int> &boneMap,
                            std::unordered_map<std::string, int> &nameToNodeIndex)
{
    if(!aNode) return;

    std::string nodeName = aNode->mName.C_Str();

    Node node = {};
    node.parentId = parentId;
    node.localTransform = AssimpMat4ToGLM(aNode->mTransformation);
    node.name = (char *)aNode->mName.C_Str();

    int nodeId = nameToNodeIndex[nodeName];
    model->nodes[nodeId] = node;

    glm::mat4 nodeTransform = parentTransform * node.localTransform;

    for(uint32 nodeMeshIndex = 0; nodeMeshIndex < aNode->mNumMeshes; nodeMeshIndex++)
    {
        uint32 meshIndex = aNode->mMeshes[nodeMeshIndex];
        model->meshToNodeId[meshIndex] = nodeId;

        aiAABB aiAABB = scene->mMeshes[meshIndex]->mAABB;
        AABB meshAABB = {AssimpVec3ToGLM(aiAABB.mMin), AssimpVec3ToGLM(aiAABB.mMax)};

        AABB transformedAABB = TransformAABB(&meshAABB, nodeTransform);
        MergeAABB(&model->aabb, &transformedAABB);
    }

    if(model->type == ModelType_Animated)
    {
        int boneId = boneMap.count(nodeName) ? boneMap[nodeName] : -1;
        if(boneId >= 0)
        {
            model->animData->skeleton.nodeToBoneId[nodeId] = boneId;
        }
    }

    for(uint32 childIndex = 0; childIndex < aNode->mNumChildren; childIndex++)
    {
        FlattenAssimpHierarchy(scene, aNode->mChildren[childIndex], model, nodeId,
                               nodeTransform, boneMap, nameToNodeIndex);
    }
}

Model *ImportModel(char *filepath, GLuint shader, uint32 flags, uint16 type, float scale)
{
    Model *result = (Model *)calloc(1, sizeof(Model));
    result->numOfMeshes = -1;
    result->type = type;

    aiSetImportPropertyFloat(aiCreatePropertyStore(), AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale);
    const aiScene *scene = aiImportFile(filepath, flags | aiProcess_GenBoundingBoxes);
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
    result->material = (MaterialPhong *)calloc(result->numOfMeshes, sizeof(MaterialPhong));
    result->meshToNodeId = (int *)calloc(result->numOfMeshes, sizeof(int));

    int numOfBones = 0;
    std::unordered_map<std::string, int> boneMap;
    for(uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];
        for(uint32 boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
        {
            aiBone *bone = mesh->mBones[boneIndex];

            std::string boneName = bone->mName.C_Str();
            if(!boneMap.count(boneName))
            {
                boneMap[boneName] = numOfBones;
                numOfBones++;
            }
        }
    }

    int numOfNodes = 0;
    std::unordered_map<std::string, int> nameToNodeIndex;
    CountNodes(scene->mRootNode, &numOfNodes, nameToNodeIndex);

    if(type == ModelType_Animated)
    {
        result->animData = (AnimatedModel *)calloc(1, sizeof(AnimatedModel));

        result->animData->skeleton.numOfBones = numOfBones;
        result->animData->skeleton.invBindPoses = (glm::mat4 *)calloc(numOfBones, sizeof(glm::mat4));
        result->animData->skeleton.boneAABBs = (AABB *)calloc(numOfBones, sizeof(AABB));
        for(int boneIndex = 0; boneIndex < numOfBones; boneIndex++)
        {
            result->animData->skeleton.boneAABBs[boneIndex].min = glm::vec3(FLT_MAX);
            result->animData->skeleton.boneAABBs[boneIndex].max = glm::vec3(-FLT_MAX);
        }

        result->animData->skeleton.nodeToBoneId = (int *)calloc(numOfNodes, sizeof(int));
        memset(result->animData->skeleton.nodeToBoneId, -1, sizeof(int) * numOfNodes);

        result->animData->numOfAnimations = scene->mNumAnimations;
        result->animData->animations = (Animation *)calloc(result->animData->numOfAnimations, sizeof(Animation));

        result->animData->numOfMatrices = numOfBones;
        result->animData->skinningMatrices = (glm::mat4 *)calloc(numOfBones, sizeof(glm::mat4));
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
            std::vector<SkinnedVertex> vertices = LoadAnimatedVerticesData(mesh, &result->animData->skeleton, boneMap);
            result->mesh[meshIndex] = CreateMesh(&vertices[0], vertices.size(), sizeof(SkinnedVertex), &indices[0],
                                                 indices.size(), skinnedVertexAttribs, ArrayCount(skinnedVertexAttribs));
        }

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        Texture diffuseTexture = ImportTextures(scene, material, dirPath, &loadedDiffusePaths, &diffuseTextures, aiTextureType_DIFFUSE);
        Texture specularTexture = ImportTextures(scene, material, dirPath, &loadedSpecularPaths, &specularTextures, aiTextureType_SPECULAR);

        MaterialPhong phongMaterial = {shader, diffuseTexture, specularTexture, {}, 32.0f};
        result->material[meshIndex] = phongMaterial;
    }

    result->numOfNodes = numOfNodes;
    result->nodes = (Node *)calloc(numOfNodes, sizeof(Node));
    result->aabb.min = glm::vec3(FLT_MAX);
    result->aabb.max = glm::vec3(-FLT_MAX);
    FlattenAssimpHierarchy((aiScene *)scene, scene->mRootNode, result, -1, glm::mat4(1.0f), boneMap, nameToNodeIndex);

    UpdateAABBCorners(&result->aabb);

    if(type == ModelType_Animated)
    {
        AnimatedModel *animData = result->animData;
        for(int i = 0; i < animData->numOfMatrices; i++)
        {
            animData->skinningMatrices[i] = glm::mat4(1.0f);
        }

        for(uint32 animationIndex = 0; animationIndex < scene->mNumAnimations; animationIndex++)
        {
            aiAnimation *anim = scene->mAnimations[animationIndex];

            Animation *animation = &animData->animations[animationIndex];
            animation->ticksPerSecond = (float)anim->mTicksPerSecond;
            animation->numOfFrames = (int)anim->mDuration;

            animation->nodeToSampleId = (int *)calloc(numOfNodes, sizeof(int));
            memset(animation->nodeToSampleId, -1, sizeof(int) * numOfNodes);

            animation->numOfSamples = anim->mNumChannels;
            animation->samples = (AnimationSample *)calloc(animation->numOfSamples, sizeof(AnimationSample));

            for(uint32 sampleIndex = 0; sampleIndex < anim->mNumChannels; sampleIndex++)
            {
                aiNodeAnim *channel = anim->mChannels[sampleIndex];
                std::string nodeName = channel->mNodeName.C_Str();

                animation->nodeToSampleId[nameToNodeIndex[nodeName]] = sampleIndex;

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

                animation->samples[sampleIndex] = sample;
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

void RenderModel(Game *game, Model *model, glm::mat4 modelMat, glm::mat4 *nodeTransforms)
{
    GLuint shader = model->material[0].shader;
    if(model->type == ModelType_Static)
    {
        if(game->outlinePass)
            shader = game->outlineShader;
        if(game->pickingPass)
            shader = game->pickingShader;
    }
    else if(model->type == ModelType_Animated)
    {
        if(game->outlinePass)
            shader = game->skinnedOutlineShader;
        if(game->pickingPass)
            shader = game->skinnedPickingShader;

        ShaderSetMatrix4Array(shader, "u_skinning", glm::value_ptr(model->animData->skinningMatrices[0]), 100);
    }

    for(int meshIndex = 0; meshIndex < model->numOfMeshes; meshIndex++)
    {
        glm::mat4 meshModelMatrix = modelMat;
        if(nodeTransforms)
        {
            meshModelMatrix = modelMat * nodeTransforms[model->meshToNodeId[meshIndex]];
        }
        RenderMesh(game, &model->mesh[meshIndex], meshModelMatrix, shader, &model->material[meshIndex]);
    }
}