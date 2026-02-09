#ifndef ANIMATION_H

struct Entity;
struct AnimatedModel;
struct AABB;
struct Model;

struct Skeleton
{
    glm::mat4 *invBindPoses;
    AABB *boneAABBs;
    int numOfBones;

    int *nodeToBoneId;
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
glm::vec3 AssimpVec3ToGLM(aiVector3D v);

void UpdateAnimation(Entity *entity, float deltaTime);

#define ANIMATION_H
#endif