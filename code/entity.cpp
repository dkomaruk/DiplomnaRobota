#include "entity.h"

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

void RenderEntity(Engine *engine, Entity *entity)
{
    Mesh *mesh = &entity->mesh;

    mat4 model = mat4(1.0f);
    model = translate(model, entity->position);

    //TODO: Rotation using quaternions
    model = rotate(model, radians(entity->rotation.x), vec3(1.0f, 0.0f, 0.0f));
    model = rotate(model, radians(entity->rotation.y), vec3(0.0f, 1.0f, 0.0f));
    model = rotate(model, radians(entity->rotation.z), vec3(0.0f, 0.0f, 1.0f));

    model = scale(model, entity->scale);

    glUseProgram(mesh->shader);

    glUniformMatrix4fv(glGetUniformLocation(mesh->shader, "u_model"), 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(mesh->shader, "u_view"), 1, GL_FALSE, value_ptr(engine->view));
    glUniformMatrix4fv(glGetUniformLocation(mesh->shader, "u_projection"), 1, GL_FALSE, value_ptr(engine->projection));

    if(mesh->texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->texture);
    }

    glBindVertexArray(mesh->vao);
    if(mesh->indicesCount > 0)
    {
        glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mesh->verticesCount);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}