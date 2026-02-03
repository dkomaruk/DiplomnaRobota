#ifndef ANIMATION_H

#include "defines.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <unordered_map>

struct AnimatedModel;

struct Node
{
    glm::mat4 localTransform;
    int parentId;
    int boneId;
};

struct Skeleton
{
    glm::mat4 *invBindPoses;
    int numOfBones;

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
    float ticksPerSecond;
    int numOfFrames;

    int *nodeToSampleId;
    int numOfSamples;
    AnimationSample *samples;
};

glm::mat4 AssimpMat4ToGLM(aiMatrix4x4 m);
void UpdateAnimation(AnimatedModel *model, float deltaTime);

#define ANIMATION_H
#endif