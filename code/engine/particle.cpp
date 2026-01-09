#include "particle.h"

#include "mesh.h"
#include "shader.h"

void RenderParticle(Particle *particle, Texture *texture, GLuint shader, float scale)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, particle->pos);
    model = glm::translate(model, glm::vec3(texture->x * scale, texture->y * scale, 0.0f) / 2.0f);
    model = glm::rotate(model, glm::radians(particle->rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, -glm::vec3(texture->x * scale, texture->y * scale, 0.0f) / 2.0f);
    model = glm::scale(model, glm::vec3(texture->x * scale, texture->y * scale, 1.0f));

    ShaderSetVec4(shader, "u_color", particle->colorOut);

    RenderSurface(shader, texture, model);
}
