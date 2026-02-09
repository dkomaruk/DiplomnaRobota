#include "animation.h"

#include "model.h"

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

glm::vec3 AssimpVec3ToGLM(aiVector3D v)
{
    return glm::vec3(v.x, v.y, v.z);
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

void UpdateAnimation(Model *model, float deltaTime)
{
    Assert(model->type == ModelType_Animated)

    if(!model->animData->numOfAnimations) return;

    model->aabb.min = glm::vec3(FLT_MAX);
    model->aabb.max = glm::vec3(-FLT_MAX);

    AnimatedModel *animData = model->animData;
    Animation *animation = &animData->animations[animData->currentAnimation];
    Skeleton *skeleton = &animData->skeleton;

    animData->time += deltaTime;
    float timeInTicks = animData->time * animation->ticksPerSecond;
    float time = fmod(timeInTicks, (float)animation->numOfFrames);

    glm::mat4 *globalTransforms = (glm::mat4 *)alloca(sizeof(glm::mat4) * model->numOfNodes);
    for(int nodeIndex = 0; nodeIndex < model->numOfNodes; nodeIndex++)
    {
        Node *node = &model->nodes[nodeIndex];

        int sampleId = animation->nodeToSampleId[nodeIndex];
        glm::mat4 nodeTransform = (sampleId != -1) ? GetInterpolatedTransform(&animation->samples[sampleId], time)
                                                   : node->localTransform;

        globalTransforms[nodeIndex] = (node->parentId != -1) ? globalTransforms[node->parentId] * nodeTransform
                                                             : nodeTransform;

        int boneId = skeleton->nodeToBoneId[nodeIndex];
        if(boneId != -1)
        {
            animData->skinningMatrices[boneId] = globalTransforms[nodeIndex] * skeleton->invBindPoses[boneId];

            if(skeleton->boneAABBs[boneId].min.x <= skeleton->boneAABBs[boneId].max.x)
            {
                AABB transformedAABB = TransformAABB(&skeleton->boneAABBs[boneId], globalTransforms[nodeIndex]);
                MergeAABB(&model->aabb, &transformedAABB);
            }
        }
    }

    glm::vec3 corners[8];
    GetAABBCorners(&model->aabb, corners);

    glBindBuffer(GL_ARRAY_BUFFER, model->meshAABB.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(corners), corners);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}