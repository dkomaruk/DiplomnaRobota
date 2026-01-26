#ifndef ANIMATION_H

#include "defines.h"

#include "mesh.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <unordered_map>

//This is 64 bytes according to sizeof(SkinnedVertex)
struct SkinnedVertex
{
    glm::ivec4 boneId;
    glm::vec4 weight;
    glm::vec3 normal;
    glm::vec3 position;
    glm::vec2 uv;
};

struct Bone
{
    glm::mat4x4 invBindPose;
    std::string name;
    int id;
};

struct Node
{
    glm::mat4 localTransform;
    std::string name;
    int boneId;
    int parentId;
};

struct Skeleton
{
    std::unordered_map<std::string, Bone> boneMap;
    int numOfBones;

    std::unordered_map<std::string, int> nameToNodeIndex;

    Node *nodes;
    int numOfNodes;
};

struct KeyPosition
{
    glm::vec3 position;
    float time;
};

struct KeyRotation
{
    glm::quat rotation;
    float time;
};

struct KeyScale
{
    glm::vec3 scale;
    float time;
};

struct AnimationSample
{
    int numOfPositions;
    KeyPosition *posKeys;

    int numOfRotations;
    KeyRotation *rotKeys;

    int numOfScalings;
    KeyScale *scaleKeys;
};

struct Animation
{
    Skeleton *skeleton;
    float ticksPerSecond;
    int numOfFrames;
    //AnimationSample *samples;
    std::unordered_map<std::string, AnimationSample> samples;
};

glm::mat4 SampleAnimation(Animation *animation, float time);
Mesh CreateMesh(std::vector<SkinnedVertex> vertices);
Mesh CreateMesh(std::vector<SkinnedVertex> vertices, std::vector<unsigned int> indices);
glm::mat4 AssimpMat4ToGLM(aiMatrix4x4 m);

#define ANIMATION_H
#endif