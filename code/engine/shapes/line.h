#ifndef LINE_H

#include "mesh.h"

#include <GL/glew.h>

#include <glm/vec2.hpp>

struct Line
{
    glm::vec3 start;
    glm::vec3 end;

    Mesh mesh;
    GLuint shader;
    glm::vec3 color;
};

Line CreateLine(glm::vec3 start, glm::vec3 end, GLuint shader, glm::vec3 color = glm::vec3(1.0f));
void RenderLine(Line *line);
void UpdateLine(Line *line, glm::vec3 start, glm::vec3 end);

#define LINE_H
#endif