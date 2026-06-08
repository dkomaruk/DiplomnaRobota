#include "asset_manager.h"

#include "game.h"
#include "shader.h"
#include "mesh.h"
#include "framebuffer.h"
#include "debug.h"
#include "debug.h"
#include "noise.h"
#include "frustum.h"

#include "string_utils.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <stb_image.h>

#include <FastNoiseLite.h>

#include <AL/al.h>
#include <AL/alext.h>

#include <stb_vorbis.c>

#include <json.hpp>

#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

Texture LoadModelTextures(const aiScene *scene, aiMaterial *material, const std::string &dirPath,
                          AssetManager *assets, const std::string &modelName, aiTextureType type)
{
    Texture result = {};

    int texturesCount = material->GetTextureCount(type);
    if(!texturesCount) return result; //TODO: Return a missing texture placeholder

    Assert(texturesCount < 2); //TODO: Handle multiple textures of the same type on one mesh

    aiString texPath;
    material->GetTexture(type, 0, &texPath);
    std::string texturePath = texPath.C_Str();

    if(texturePath[0] == '*')
    {
        std::string key = modelName + "_embedded_" + texturePath.substr(1);

        if(assets->textures.find(key) != assets->textures.end())
            return GetTexture(assets, key);

        int index = atoi(texturePath.c_str() + 1);
        aiTexture *texture = scene->mTextures[index];

        if(texture->mHeight == 0)
        {
            int w, h;
            u8 *image = stbi_load_from_memory((u8 *)texture->pcData, texture->mWidth, &w, &h, 0, 4);
            if(!image) return result;

            result = CreateGLTexture(image, w, h);
            stbi_image_free(image);
        }
        else
        {
            result = CreateGLTexture((u8 *)texture->pcData, texture->mWidth, texture->mHeight);
        }

        assets->textures[key] = result;
    }
    else
    {
        texturePath = dirPath + '/' + texturePath;
        result = GetTextureByPath(assets, texturePath);
    }

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
    for(u32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        vertices[vertexIndex].boneId = glm::ivec4(-1);
        LoadVertexData(mesh, vertexIndex, &vertices[vertexIndex].vertex);
    }

    //Load bones
    for(u32 boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
    {
        aiBone *aBone = mesh->mBones[boneIndex];

        std::string boneName = aBone->mName.C_Str();
        int boneId = boneMap[boneName];

        skeleton->invBindPoses[boneId] = AssimpMat4ToGLM(aBone->mOffsetMatrix);

        Assert(boneId >= 0 && boneId < skeleton->numOfBones)

        //Load bone weights into vertices
        for(u32 weightIndex = 0; weightIndex < aBone->mNumWeights; weightIndex++)
        {
            int vertexId = aBone->mWeights[weightIndex].mVertexId;
            Assert((u32)vertexId < mesh->mNumVertices)

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

    for(u32 vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
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

    for(u32 childIndex = 0; childIndex < node->mNumChildren; childIndex++)
    {
        CountNodes(node->mChildren[childIndex], counter, nameToNodeIndex);
    }
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

    for(u32 nodeMeshIndex = 0; nodeMeshIndex < aNode->mNumMeshes; nodeMeshIndex++)
    {
        u32 meshIndex = aNode->mMeshes[nodeMeshIndex];
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

    for(u32 childIndex = 0; childIndex < aNode->mNumChildren; childIndex++)
    {
        FlattenAssimpHierarchy(scene, aNode->mChildren[childIndex], model, nodeId,
                               nodeTransform, boneMap, nameToNodeIndex);
    }
}

Model *LoadModel(AssetManager *assets, const std::string &filepath, const std::string &modelName,
                 GLuint shader, u32 flags, u16 type, float scale)
{
    Model *result = (Model *)calloc(1, sizeof(Model));
    result->numOfMeshes = -1;

    aiPropertyStore* props = aiCreatePropertyStore();
    aiSetImportPropertyFloat(props, AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale);
    const aiScene *scene = aiImportFileExWithProperties(filepath.c_str(), flags | aiProcess_GenBoundingBoxes, 0, props);
    aiReleasePropertyStore(props);

    if(!scene)
    {
        SDL_Log("Failed to load %s. Error: %s", filepath.c_str(), aiGetErrorString());
        return result;
    }

    if(type == ModelType_DetermineOnLoad)
        result->type = (u16)((scene->mNumAnimations == 0) ? ModelType_Static : ModelType_Animated);
    else
        result->type = type;

    result->numOfMeshes = scene->mNumMeshes;
    result->mesh = (Mesh *)calloc(result->numOfMeshes, sizeof(Mesh));
    result->material = (MaterialPhong *)calloc(result->numOfMeshes, sizeof(MaterialPhong));
    result->meshToNodeId = (int *)calloc(result->numOfMeshes, sizeof(int));

    //Count bones and nodes.
    //Each unique bone name is mapped to an index in the bone array because different meshes can refer to the same bones.
    int numOfBones = 0;
    std::unordered_map<std::string, int> boneMap;
    for(u32 meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];
        for(u32 boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
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

    //Initialize animation data memory using bone and node counts
    if(result->type == ModelType_Animated)
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
    }

    //Parse mesh data and textures out of assimp data structures
    std::string dirPath = filepath;
    size_t found = dirPath.find_last_of("\\/");
    dirPath = (found == std::string::npos) ? dirPath : dirPath.substr(0, found);

    for(u32 meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh *mesh = scene->mMeshes[meshIndex];
        bool hasUVs = mesh->HasTextureCoords(0);

        std::vector<u32> indices;
        for(u32 j = 0; j < mesh->mNumFaces; j++)
        {
            for(u32 k = 0; k < mesh->mFaces[j].mNumIndices; k++)
            {
                indices.push_back(mesh->mFaces[j].mIndices[k]);
            }
        }

        if(result->type == ModelType_Static)
        {
            std::vector<Vertex> vertices = LoadStaticVerticesData(mesh);
            result->mesh[meshIndex] = CreateMesh(&vertices[0], vertices.size(), sizeof(Vertex),
                                                 &indices[0], indices.size());
        }
        else if(result->type == ModelType_Animated)
        {
            std::vector<SkinnedVertex> vertices = LoadAnimatedVerticesData(mesh, &result->animData->skeleton, boneMap);
            result->mesh[meshIndex] = CreateMesh(&vertices[0], vertices.size(), sizeof(SkinnedVertex), &indices[0],
                                                 indices.size(), skinnedVertexAttribs, ArrayCount(skinnedVertexAttribs));
        }

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        Texture diffuseTexture = LoadModelTextures(scene, material, dirPath, assets, modelName, aiTextureType_DIFFUSE);
        Texture specularTexture = LoadModelTextures(scene, material, dirPath, assets, modelName, aiTextureType_SPECULAR);

        if(type == ModelType_DetermineOnLoad)
            shader = (result->type == ModelType_Static) ? GetShader(assets, "main") : GetShader(assets, "animation");

        MaterialPhong phongMaterial = {shader, diffuseTexture, specularTexture, {}, 32.0f};
        result->material[meshIndex] = phongMaterial;
    }

    //Flatten Assimp hierarchy tree into a flat array.
    //This is done so that there is no need to traverse the tree every time
    //an animation has to be updated and instead a simple array can be iterated.
    result->numOfNodes = numOfNodes;
    result->nodes = (Node *)calloc(numOfNodes, sizeof(Node));
    result->aabb.min = glm::vec3(FLT_MAX);
    result->aabb.max = glm::vec3(-FLT_MAX);
    FlattenAssimpHierarchy((aiScene *)scene, scene->mRootNode, result, -1, glm::mat4(1.0f), boneMap, nameToNodeIndex);

    UpdateAABBCorners(&result->aabb);

    //Parse animation data out of Assimp data structures
    if(result->type == ModelType_Animated)
    {
        AnimatedModel *animData = result->animData;

        for(u32 animationIndex = 0; animationIndex < scene->mNumAnimations; animationIndex++)
        {
            aiAnimation *anim = scene->mAnimations[animationIndex];

            Animation *animation = &animData->animations[animationIndex];
            animation->ticksPerSecond = (float)anim->mTicksPerSecond;
            animation->numOfFrames = (int)anim->mDuration;

            animation->nodeToSampleId = (int *)calloc(numOfNodes, sizeof(int));
            memset(animation->nodeToSampleId, -1, sizeof(int) * numOfNodes);

            animation->numOfSamples = anim->mNumChannels;
            animation->samples = (AnimationSample *)calloc(animation->numOfSamples, sizeof(AnimationSample));

            for(u32 sampleIndex = 0; sampleIndex < anim->mNumChannels; sampleIndex++)
            {
                aiNodeAnim *channel = anim->mChannels[sampleIndex];
                std::string nodeName = channel->mNodeName.C_Str();

                animation->nodeToSampleId[nameToNodeIndex[nodeName]] = sampleIndex;

                AnimationSample sample = {};

                sample.numOfPositions = channel->mNumPositionKeys;
                sample.posKeys = (KeyPosition *)calloc(channel->mNumPositionKeys, sizeof(KeyPosition));
                for(u32 posKeyIndex = 0; posKeyIndex < channel->mNumPositionKeys; posKeyIndex++)
                {
                    aiVectorKey *pk = &channel->mPositionKeys[posKeyIndex];
                    sample.posKeys[posKeyIndex].time = (float)pk->mTime;
                    sample.posKeys[posKeyIndex].position = glm::vec3(pk->mValue.x, pk->mValue.y, pk->mValue.z);
                }

                sample.numOfRotations = channel->mNumRotationKeys;
                sample.rotKeys = (KeyRotation *)calloc(channel->mNumRotationKeys, sizeof(KeyRotation));
                for(u32 rotKeyIndex = 0; rotKeyIndex < channel->mNumRotationKeys; rotKeyIndex++)
                {
                    aiQuatKey *rk = &channel->mRotationKeys[rotKeyIndex];
                    sample.rotKeys[rotKeyIndex].time = (float)rk->mTime;
                    sample.rotKeys[rotKeyIndex].rotation = glm::quat(rk->mValue.w, rk->mValue.x,
                                                                    rk->mValue.y, rk->mValue.z);
                }

                sample.numOfScalings = channel->mNumScalingKeys;
                sample.scaleKeys = (KeyScale *)calloc(channel->mNumScalingKeys, sizeof(KeyScale));
                for(u32 scaleKeyIndex = 0; scaleKeyIndex < channel->mNumScalingKeys; scaleKeyIndex++)
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

#define TEXTURE_ATLAS_ELEMENT 0
#define SPRITE_ELEMENT 1

void XMLCALL ParseParticleSettingsStartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
    Atlas *atlas = (Atlas *)userData;

    int element = -1;
    if(strcmp(name, "TextureAtlas") == 0)
    {
        element = TEXTURE_ATLAS_ELEMENT;
    }
    else if(strcmp(name, "sprite") == 0)
    {
        element = SPRITE_ELEMENT;
    }

    switch(element)
    {
        case TEXTURE_ATLAS_ELEMENT:
        {
            atlas->size.x = StrToFloat(atts[3]);
            atlas->size.y = StrToFloat(atts[5]);
            //SDL_Log("x: %f; y:%f", atlas->size.x, atlas->size.y);
        } break;

        case SPRITE_ELEMENT:
        {
            Sprite sprite = {};
            float pixelX = StrToFloat(atts[3]);
            float pixelY = StrToFloat(atts[5]);
            float pixelW = StrToFloat(atts[7]);
            float pixelH = StrToFloat(atts[9]);

            sprite.pos.x = pixelX / atlas->size.x;
            sprite.pos.y = (atlas->size.y - pixelY - pixelH) / atlas->size.y;

            sprite.size.x = pixelW / atlas->size.x;
            sprite.size.y = pixelH / atlas->size.y;

            atlas->sprites.push_back(sprite);

            //SDL_Log("%d. (%f, %f), (%f, %f)", (int)atlas->sprites.size(), sprite.pos.x, sprite.pos.y, sprite.size.x, sprite.size.y);
        } break;
    }
}

void XMLCALL ParseParticleSettingsEndElement(void *userData, const XML_Char *name) { }

void LoadParticleSystem(Game *game)
{
    size_t fileSize;
    void *fileMemory = SDL_LoadFile("../data/textures/particles/animated/1.xml", &fileSize);
    if(!fileMemory)
    {
        SDL_Log("Failed to load 1.xml. Error: %s", SDL_GetError());
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    if(!parser)
    {
        SDL_Log("Failed to create an XML parser");
    }

    XML_SetUserData(parser, (void *)&game->atlas);
    XML_SetElementHandler(parser, ParseParticleSettingsStartElement, ParseParticleSettingsEndElement);

    XML_Status parsingResult = XML_Parse(parser, (char *)fileMemory, (int)fileSize, XML_TRUE);
    if(!parsingResult)
    {
        SDL_Log("Failed to parse 1.xml");
    }
    SDL_free(fileMemory);

    AssetManager *assets = &game->assets;
    game->particleTextures[0] = GetTexture(assets, "smoke");
    game->particleTextures[1] = GetTexture(assets, "smoke2");
    game->particleTextures[2] = GetTexture(assets, "smoke3");
    game->particleTextures[3] = GetTexture(assets, "smoke4");
    game->particleTextures[4] = GetTexture(assets, "smoke5");
    game->particleTextures[5] = GetTexture(assets, "animated_smoke");
    game->particleTextures[6] = GetTexture(assets, "fire");
    game->particleTextures[7] = GetTexture(assets, "fire2");
    game->particleTextures[8] = GetTexture(assets, "circle_05_a");
    game->particleTextures[9] = GetTexture(assets, "twirl_04_a");
    game->particleTextures[10] = GetTexture(assets, "star_05_a");
    game->particleTextures[11] = GetTexture(assets, "effect_02_a");
    game->particleTextures[12] = GetTexture(assets, "trace_01_a");

    for(int i = 0; i < ArrayCount(game->particleSystems); i++)
    {
        game->particleSystems[i] = InitParticleSystem(game, &game->smokeSettings);
    }

    game->particleSystems[0].pos = glm::vec3(0.0f);

    int maxNumOfParticles = game->smokeSettings.maxNumOfParticles;
    game->particleData = (ParticleData *)calloc(maxNumOfParticles * ArrayCount(game->particleSystems), sizeof(ParticleData));
    game->textureID = game->particleTextures[game->currentTexture].id;

    game->atlas.path = "../data/textures/particles/animated/animated_smoke.png";
    game->smokeSettings.atlas = &game->atlas;

    game->particlesQuad = CreateUnitQuadStripes();
    glBindVertexArray(game->particlesQuad.vao);

    glGenBuffers(1, &game->vboInstances);
    glBindBuffer(GL_ARRAY_BUFFER, game->vboInstances);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * maxNumOfParticles * ArrayCount(game->particleSystems),
                 game->particleData, GL_STREAM_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, scale));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, angle));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, uvOffset));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, uvScale));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, offset));
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (void *)offsetof(ParticleData, color));

    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for(int i = 0; i < ArrayCount(game->particleSystems); ++i)
    {
        if(game->smokeSettings.prewarm)
        {
            game->particleSystems[i].prewarmTimer = StartTimer(game->smokeSettings.prewarmSeconds);
        }
    }
}

Entity *AddNewEntityToScene(Game *game, Model *model, char *modelName, char *textId, glm::vec3 position,
                            glm::vec3 rotation, glm::vec3 scale)
{
    Entity *newEntity = (Entity *)calloc(1, sizeof(Entity));
    *newEntity = CreateEntity(model);

    newEntity->position = position;
    newEntity->rotation = rotation;
    newEntity->scale = scale;

    strcpy(newEntity->textId, textId);
    strncpy(newEntity->modelName, modelName, sizeof(newEntity->modelName) - 1);
    newEntity->modelName[sizeof(newEntity->modelName) - 1] = '\0';

    newEntity->id = game->sceneEntities.size() ? (game->sceneEntities.back()->id + 1) : 1;
    game->sceneEntities.push_back(newEntity);

    return newEntity;
}

std::string RegisterAsset(AssetManager *assets, const fs::path &path)
{
    std::string ext = path.extension().string();
    std::string name = path.stem().string();
    std::string fullPath = path.string();

    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) -> char { return (char)std::tolower(c); });

    if(ext == ".fbx" || ext == ".obj" || ext == ".gltf")
        assets->modelPaths[name] = fullPath;
    else if(ext == ".png" || ext == ".jpg" || ext == ".bmp")
        assets->texturePaths[name] = fullPath;

    return name;
}

void from_json(const json &j, ShaderDefinition &def)
{
    def.vertex = j.value("vertex", "");
    def.fragment = j.value("fragment", "");
    def.tess_control = j.value("tess_control", "");
    def.tess_eval = j.value("tess_eval", "");
}

void LoadShaderManifest(AssetManager *assets, const std::string &manifestPath)
{
    std::ifstream file(manifestPath);
    if(!file.is_open())
    {
        SDL_Log("Failed to open global shader manifest: %s", manifestPath.c_str());
        return;
    }

    json manifestJson;
    file >> manifestJson;

    Game *game = GetGame();
    for(auto &[shaderName, shaderStages] : manifestJson.items())
    {
        assets->shaderManifests[shaderName] = shaderStages.get<ShaderDefinition>();
        GetShader(game, shaderName);
    }
}

void ParseAssetsDirectories(AssetManager *assets, const std::string &baseDir)
{
    assets->baseDir = baseDir;
    assets->shadersDir = baseDir + "/shaders";

    LoadShaderManifest(assets, assets->shadersDir + "/shaders.json");

    if(fs::exists(baseDir) && fs::is_directory(baseDir))
    {
        for(const auto& entry : fs::recursive_directory_iterator(baseDir))
        {
            if(entry.is_regular_file())
            {
                RegisterAsset(assets, entry.path());
            }
        }
    }
    else
    {
        SDL_Log("Data directory not found: %s", baseDir.c_str());
    }
}

Model *GetModel(AssetManager *assets, const std::string &name, GLuint shader, u16 type, u32 flags, float scale)
{
    if(assets->models.find(name) != assets->models.end())
        return assets->models[name];

    if(assets->modelPaths.find(name) != assets->modelPaths.end())
    {
        Model* newModel = LoadModel(assets, (char*)assets->modelPaths[name].c_str(),
                                    name, shader, aiProcess_Triangulate | flags, type, scale);
        assets->models[name] = newModel;
        return newModel;
    }

    SDL_Log("Model not found in registry: %s", name.c_str());
    return nullptr;
}

Texture GetTexture(AssetManager *assets, const std::string &name, u32 flags)
{
    if(assets->textures.find(name) != assets->textures.end())
        return assets->textures[name];

    if(assets->texturePaths.find(name) != assets->texturePaths.end())
    {
        Texture tex = CreateTexture((char*)assets->texturePaths[name].c_str(), flags);
        assets->textures[name] = tex;
        return tex;
    }

    SDL_Log("Texture not found in registry: %s", name.c_str());
    return {};
}

Texture GetTextureByPath(AssetManager *assets, const std::string &path)
{
    if(assets->textures.find(path) != assets->textures.end())
        return assets->textures[path];

    Texture texture = CreateTexture((char*)path.c_str());
    assets->textures[path] = texture;

    return texture;
}

GLuint GetShader(AssetManager *assets, const std::string &name)
{
    if(assets->shaders.find(name) != assets->shaders.end())
        return assets->shaders[name];

    if(assets->shaderManifests.find(name) != assets->shaderManifests.end())
    {
        ShaderDefinition &def = assets->shaderManifests[name];
        char *vertShader = 0, *fragShader = 0, *tescShader = 0, *teseShader = 0;

        std::string path = assets->shadersDir + "/";

        if(!def.vertex.empty())
            vertShader = LoadShader((path + def.vertex).c_str());
        if(!def.fragment.empty())
            fragShader = LoadShader((path + def.fragment).c_str());

        if(!def.tess_control.empty() && !def.tess_eval.empty())
        {
            tescShader = LoadShader((path + def.tess_control).c_str());
            teseShader = LoadShader((path + def.tess_eval).c_str());
        }

        GLuint shaderProg = CreateShaderProgram(vertShader, fragShader, tescShader, teseShader);

        assets->shaders[name] = shaderProg;
        return shaderProg;
    }

    SDL_Log("Shader not defined in manifest: %s", name.c_str());
    return 0;
}

GLuint GetShader(Game *game, const std::string &name)
{
    return GetShader(&game->assets, name);
}

void LoadTestScene(Game *game)
{
    AssetManager *assets = &game->assets;
    ParseAssetsDirectories(assets, "../data");

    //FONTS
    //int fontSizes[] = {4, 12, 18, 20, 24, 36, 48};
    int fontSizes[] = {48};
    int numOfFonts = sizeof(fontSizes) / sizeof(int);

    for(int i = 0; i < numOfFonts; i++)
    {
        int fontSize = fontSizes[i];

        game->fonts[fontSize] = PrepareFont("../data/fonts/Roboto-Regular.ttf", fontSize);
        if(!game->fonts[fontSize].ttfFont)
        {
            SDL_Log("Failed to load Roboto-Regular.ttf font%d. Error: %s", fontSize, SDL_GetError());
        }
    }

    //SHADERS
    std::unordered_map<std::string, GLuint> &shaders = assets->shaders;

    ShaderSetVec2(GetShader(game, "main"), "u_viewport", game->windowSize);
    ShaderSetVec2(GetShader(game, "animation"), "u_viewport", game->windowSize);

    ShaderSetVec4(GetShader(game, "light_source"), "u_color", glm::vec4(1.0f));
    ShaderSetVec4(GetShader(game, "outline"), "u_color", glm::vec4(1.0f));
    ShaderSetVec4(GetShader(game, "skinned_outline"), "u_color", glm::vec4(1.0f));
    ShaderSetVec4(GetShader(game, "selection_box"), "u_color", glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));

    ShaderSetInt(GetShader(game, "post_process"), "u_outlineThickness", (int)game->outlineThickness);
    ShaderSetInt(GetShader(game, "post_process"), "u_inverted", 0);
    ShaderSetInt(GetShader(game, "post_process"), "u_grayscale", 0);
    ShaderSetInt(GetShader(game, "post_process"), "u_showOutline", 1);
    ShaderSetInt(GetShader(game, "post_process"), "u_showParticles", 1);

    for(auto &shader : game->assets.shaders)
    {
        ShaderSetMatrix4(shader.second, "u_projection", game->perspectiveProjection);
    }

    ShaderSetMatrix4(GetShader(game, "ui_text"), "u_projection", game->orthoProjection);
    ShaderSetMatrix4(GetShader(game, "selection_box"), "u_projection", game->orthoProjection);

    //MESHES
    Model *abrams = GetModel(assets, "abrams", GetShader(game, "main"), ModelType_Static);
    Model *dancingModel = GetModel(assets, "Dancing", GetShader(game, "animation"),
                                   ModelType_Animated, aiProcess_GlobalScale, 1.0f);
    Model *runningModel = GetModel(assets, "Running", GetShader(game, "animation"),
                                   ModelType_Animated, aiProcess_GlobalScale, 1.0f);

    game->dancingEntity = AddNewEntityToScene(game, dancingModel, "Dancing", "dancing_entity", glm::vec3(0.0f, 0.5f, 0.0f));
    game->tank = AddNewEntityToScene(game, abrams, "abrams", "tank");
    game->runningEntity = AddNewEntityToScene(game, runningModel, "Running", "running_entity");

    Entity *tank = game->tank;
    for(int i = 0; i < tank->model->numOfNodes; i++)
    {
        char *nodeName = tank->model->nodes[i].name;
        if(nodeName && strcmp(nodeName, "Tourelle_01") == 0)
        {
            tank->turret.nodeId = i;
            tank->turret.transform = tank->model->nodes[i].localTransform;
        }
        else if(nodeName && strcmp(nodeName, "Axe_Canon_01") == 0)
        {
            tank->gun.nodeId = i;
            tank->gun.transform = tank->model->nodes[i].localTransform;
        }
        else if(nodeName && strcmp(nodeName, "Fx_Tourelle1_Tir_01") == 0)
        {
            tank->gunTip.nodeId = i;
            tank->gunTip.transform = tank->model->nodes[i].localTransform;
        }
    }

    //Scene Light
    glm::vec3 dirDiffuse = glm::vec3(0.9f);
    glm::vec3 dirAmbient = glm::vec3(0.4f);
    glm::vec3 dirSpecular = glm::vec3(1.0f);
    game->dirLight = CreateDirLight(glm::vec3(1.3f, -2.3f, -0.0f), dirDiffuse, dirAmbient, dirSpecular);
    ShaderSetDirLight(GetShader(game, "main"), game->dirLight);
    ShaderSetDirLight(GetShader(game, "animation"), game->dirLight);
    ShaderSetDirLight(GetShader(game, "terrain"), game->dirLight);
    ShaderSetDirLight(GetShader(game, "tessellated_terrain"), game->dirLight);
    ShaderSetInt(GetShader(game, "main"), "u_dirLightCount", 1);
    ShaderSetInt(GetShader(game, "animation"), "u_dirLightCount", 1);

    ShaderSetInt(GetShader(game, "main"), "u_pointLightCount", 0);
    ShaderSetInt(GetShader(game, "animation"), "u_pointLightCount", 0);

    //Terrain
    glm::vec2 terrainSize = glm::vec2(1024, 1024);
    TerrainGenerationSettings settings = { .maxHeight = 20.0f, .yOffset = 22.0f };

    float *perlinNoise = GeneratePerlinNoise2(terrainSize, settings.gridSize, settings.octaves,
                                                settings.persistence, settings.lacunarity);
    float *heightmap = GenerateTerrainHeightmap(perlinNoise, terrainSize, &settings);
    UpdateTerrainEditorHeightmap(game, perlinNoise, terrainSize);

    game->terrain = CreateTessellatedTerrainMesh(heightmap, terrainSize, GetShader(game, "tessellated_terrain"),
                                                 &settings);
    game->terrain.colorTexture = GetTexture(assets, "wispy-grass-meadow_albedo");

    //Particles
    LoadParticleSystem(game);

    //Skymap
    int flags = TexturePreset_Common;
    flags = FLAG_TOGGLE(flags, TextureFlag_Filter_Min_LinLin | TextureFlag_Filter_Min_Nearest | TextureFlag_FlipY);
    game->skymapTexture = GetTexture(assets, "sky", flags);

    //Debug lines
    game->pickingRay = CreateLine(glm::vec3(0.0f), glm::vec3(0.0f), GetShader(game, "line"), glm::vec3(1.0f, 0.0f, 0.0f));
    CreateFrustumLines(game->frustumLines, game->frustumNormals, GetShader(game, "line"));

    //Shadow volume lines
    CreateShadowVolumeLines(game->shadowVolume, GetShader(game, "line"));
    game->dirLightView = lookAt(-game->dirLight.direction * game->shadowVolumeOffset,
                                 glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightViewProj = game->orthoProjDirLight * game->dirLightView;
    UpdateShadowVolumeLines(game->shadowVolume, lightViewProj);

    //Text
    glm::vec3 textColor = glm::vec3(1.0f, 0.0f, 0.0f);
    game->fpsCounter = CreateText(&game->fonts[48], "0 FPS", glm::vec2(20.0f, 54.0f), GetShader(game, "ui_text"), textColor);
    game->msPerFrame = CreateText(&game->fonts[48], "0 ms/f", glm::vec2(400.0f, 54.0f), GetShader(game, "ui_text"), textColor);
    game->aliveParticlesText = CreateText(&game->fonts[48], "Alive Particles: 0", glm::vec2(20.0f, 54.0f + 60.0f), GetShader(game, "ui_text"), textColor);
    game->deadParticlesText = CreateText(&game->fonts[48], "Dead Particles: 0", glm::vec2(20.0f, 54.0f + 60.0f + 60.0f), GetShader(game, "ui_text"), textColor);

    game->fullscreenQuad = CreateQuadNDC(glm::vec2(0.0f), glm::vec2(game->windowSize), glm::vec2(game->windowSize));
}