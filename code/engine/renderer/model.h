#ifndef MODEL_H

#include "defines.h"

#include "aabb.h"
#include "animation.h"

#include <GL/glew.h>

#include <glm/vec3.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Game;
struct Mesh;
struct MaterialPhong;

struct AnimatedModel
{
    Skeleton skeleton;

    Animation *animations;
    int numOfAnimations;
};

struct StaticModel { };

enum ModelType
{
    ModelType_Static,
    ModelType_Animated,
    ModelType_DetermineOnLoad
};

struct Node
{
    glm::mat4 localTransform;
    int parentId;
    char *name;
};

struct Model
{
    u16 type;

    MaterialPhong *material;
    Mesh *mesh;
    int numOfMeshes;

    int *meshToNodeId;

    Node *nodes;
    int numOfNodes;

    AABB aabb;

    union
    {
        StaticModel *staticData;
        AnimatedModel *animData;
    };
};

Model *ImportModel(char *filepath, GLuint shader, u32 flags = 0, u16 type = ModelType_Static, float scale = 1.0f);

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
void RenderModel(Game *game, Model *model, glm::mat4 modelMat, glm::mat4 *nodeTransforms = 0,
                 glm::mat4 *skinningMatrices = 0, int numOfMatrices = 0);

#define MODEL_H
#endif