#include "ui.h"

#include "game.h"

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

void RenderRectUI(Game *game, glm::vec2 pos, glm::vec2 size, GLuint shader)
{
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(pos.x + (size.x / 2.0f), pos.y + (size.y / 2.0f), 0.0f));
    modelMat = glm::scale(modelMat, glm::vec3(size.x, size.y, 1.0f));

    RenderMesh(game, GetUnitQuad(), modelMat, shader);
}
