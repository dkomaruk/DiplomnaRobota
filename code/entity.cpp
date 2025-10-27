#include "entity.h"

#include "graphics/shader.h"
#include "graphics/texture.h"

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Entity CreateEntity(Mesh mesh)
{
    Entity entity = {};
    entity.mesh = mesh;

    return entity;
}

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

    UseShader(mesh->shader);

    //mat3 normalMatrix = mat3(transpose(inverse(engine->view * model)));
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    glUniformMatrix3fv(glGetUniformLocation(mesh->shader, "u_normalMatrix"), 1, GL_FALSE, value_ptr(normalMatrix));

    glUniformMatrix4fv(glGetUniformLocation(mesh->shader, "u_model"), 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(mesh->shader, "u_view"), 1, GL_FALSE, value_ptr(engine->view));
    glUniformMatrix4fv(glGetUniformLocation(mesh->shader, "u_projection"), 1, GL_FALSE, value_ptr(engine->projection));

    MaterialPhong mat = mesh->material;
    ShaderSetVec3(mesh->shader, "u_material.diffuse", mat.diffuse);
    ShaderSetVec3(mesh->shader, "u_material.ambient", mat.ambient);
    ShaderSetVec3(mesh->shader, "u_material.specular", mat.specular);
    ShaderSetFloat(mesh->shader, "u_material.shininess", mat.shininess);

    if(mesh->texture)
    {
        SetTexture(mesh->texture, 0);
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