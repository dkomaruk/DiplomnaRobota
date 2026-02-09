#ifndef MODEL_H

#include "defines.h"

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
    float time;

    Skeleton skeleton;

    Animation *animations;
    int numOfAnimations;
    int currentAnimation;

    glm::mat4 *skinningMatrices;
    int numOfMatrices;
};

struct StaticModel { };

enum ModelType
{
    ModelType_Static,
    ModelType_Animated
};

struct Node
{
    glm::mat4 localTransform;
    int parentId;
    char *name;
};

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 corners[8];
};

struct Model
{
    uint16 type;

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

Model *ImportModel(char *filepath, GLuint shader, uint32 flags = 0, uint16 type = ModelType_Static, float scale = 1.0f);

glm::mat4 PrepareModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
void RenderModel(Game *game, Model *model, glm::mat4 modelMat, glm::mat4 *nodeTransforms = NULL);

AABB TransformAABB(AABB *aabb, glm::mat4 transform);
void MergeAABB(AABB *dest, AABB *src);
void ExpandAABB(AABB *aabb, glm::vec3 point);
void UpdateAABBCorners(AABB *aabb);
void UpdateAABBMesh(AABB *aabb, Mesh *aabbMesh, bool recalculateCorners = false);

#define MODEL_H
#endif