#ifndef MODEL_H

#include "defines.h"

#include "animation.h"

#include <glm/vec3.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Mesh;

struct AnimatedModel
{
    float time;

    Skeleton *skeleton;

    Animation *animations;
    int numOfAnimations;
    int currentAnimation;

    glm::mat4 *skinningMatrices;
    int numOfMatrices;

    aiScene *scene; //TODO: Parse node hierarchy into my own structures to avoid keeping assimp data
};

struct StaticModel {};

enum ModelType
{
    ModelType_Static,
    ModelType_Animated
};

struct Model
{
    uint16 type;
    Mesh *mesh;
    MaterialPhong *material;
    int numOfMeshes;

    union
    {
        StaticModel staticData;
        AnimatedModel animData;
    };
};

Model *ImportModel(char *filepath, GLuint shader, uint32 flags = 0, uint16 type = ModelType_Static, float scale = 1.0f);

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
void RenderModel(Game *game, Model *model, glm::mat4 modelMat);

#define MODEL_H
#endif