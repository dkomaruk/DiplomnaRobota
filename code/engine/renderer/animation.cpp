#include "animation.h"

/*
void CountNodes(aiNode *node, int *counter)
{
    if(!node) return;

    *counter += 1;

    for(uint32 childIndex = 0; childIndex < node->mNumChildren; childIndex++)
    {
        CountNodes(node->mChildren[childIndex], counter);
    }
}
*/

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

int GetPosKeyIndex(float time, AnimationSample *sample)
{
    for(int i = 0; i < sample->numOfPositions - 1; i++)
    {
        if(time < sample->posKeys[i + 1].time) return i;
    }
    return 0;
}

int GetRotKeyIndex(float time, AnimationSample *sample)
{
    for(int i = 0; i < sample->numOfRotations - 1; i++)
    {
        if(time < sample->rotKeys[i + 1].time) return i;
    }
    return 0;
}

int GetScaleKeyIndex(float time, AnimationSample *sample)
{
    for(int i = 0; i < sample->numOfScalings - 1; i++)
    {
        if(time < sample->scaleKeys[i + 1].time) return i;
    }
    return 0;
}

glm::mat4 GetInterpolatedTransform(AnimationSample* sample, float time)
{
    glm::vec3 translation(0.0f);
    if(sample->numOfPositions == 1)
    {
        translation = sample->posKeys[0].position;
    }
    else if(sample->numOfPositions > 0)
    {
        int i = GetPosKeyIndex(time, sample);
        int nextI = i + 1;
        float factor = (time - sample->posKeys[i].time) / (sample->posKeys[nextI].time - sample->posKeys[i].time);
        translation = glm::mix(sample->posKeys[i].position, sample->posKeys[nextI].position, factor);
    }

    glm::quat rotation(1, 0, 0, 0);
    if(sample->numOfRotations == 1)
    {
        rotation = sample->rotKeys[0].rotation;
    }
    else if(sample->numOfRotations > 0)
    {
        int i = GetRotKeyIndex(time, sample);
        int nextI = i + 1;
        float factor = (time - sample->rotKeys[i].time) / (sample->rotKeys[nextI].time - sample->rotKeys[i].time);
        rotation = glm::slerp(sample->rotKeys[i].rotation, sample->rotKeys[nextI].rotation, factor);
    }

    glm::vec3 scale(1.0f);
    if(sample->numOfScalings == 1)
    {
        scale = sample->scaleKeys[0].scale;
    }
    else if(sample->numOfScalings > 0)
    {
        int i = GetScaleKeyIndex(time, sample);
        int nextI = i + 1;
        float factor = (time - sample->scaleKeys[i].time) / (sample->scaleKeys[nextI].time - sample->scaleKeys[i].time);
        scale = glm::mix(sample->scaleKeys[i].scale, sample->scaleKeys[nextI].scale, factor);
    }

    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
}

void ProcessNode(float time, aiNode *aiNode, AnimatedModel *model, glm::mat4 parentTransform)
{
    Animation *animation = &model->animations[model->currentAnimation];
    Skeleton *skeleton = model->skeleton;

    glm::mat4 nodeTransform = AssimpMat4ToGLM(aiNode->mTransformation);

    std::string nodeName = aiNode->mName.C_Str();
    if(animation->samples.count(nodeName))
    {
        nodeTransform = GetInterpolatedTransform(&animation->samples[nodeName], time);
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if(skeleton->boneMap.count(nodeName))
    {
        Bone *bone = &skeleton->boneMap[nodeName];
        model->skinningMatrices[bone->id] = globalTransform * bone->invBindPose;
    }

    for(uint32 childIndex = 0; childIndex < aiNode->mNumChildren; childIndex++)
    {
        ProcessNode(time, aiNode->mChildren[childIndex], model, globalTransform);
    }
}

void UpdateAnimation(AnimatedModel *model, float deltaTime)
{
    model->time += deltaTime;

    Animation *currentAnimation = &model->animations[model->currentAnimation];
    float timeInTicks = model->time * currentAnimation->ticksPerSecond;
    float animationTime = fmod(timeInTicks, (float)currentAnimation->numOfFrames);

    ProcessNode(animationTime, model->scene->mRootNode, model, glm::mat4(1.0f));
}