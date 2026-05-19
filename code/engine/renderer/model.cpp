#include "model.h"

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "game.h"
#include "debug.h"

#include <SDL3/SDL.h>

#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

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

void RenderModel(Game *game, Model *model, glm::mat4 modelMat, glm::mat4 *nodeTransforms,
                 glm::mat4 *skinningMatrices, int numOfMatrices)
{
    GLuint shader = model->material[0].shader;
    if(game->outlinePass)
    {
        shader = (model->type == ModelType_Static) ? game->assets.shaders["outline"] : game->assets.shaders["skinned_outline"];
    }
    if(game->shadowPass)
    {
        shader = game->assets.shaders["shadow"];
    }

    if(model->type == ModelType_Animated)
    {
        if(game->shadowPass)
        {
            shader = game->assets.shaders["skinned_shadow"];
        }

        ShaderSetMatrix4Array(shader, "u_skinning", glm::value_ptr(skinningMatrices[0]), numOfMatrices);
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