#include "line.h"

#include "shader.h"

Line CreateLine(glm::vec3 start, glm::vec3 end, GLuint shader, glm::vec3 color)
{
    std::vector<float> vertices;
    vertices.resize(6);
    vertices[0] = start.x; vertices[1] = start.y; vertices[2] = start.z;
    vertices[3] = end.x; vertices[4] = end.y; vertices[5] = end.z;

    AttribInfo attrib = {0, 3, GL_FLOAT, sizeof(float) * 3, (void *)0};
    Line line = {start, end, CreateMesh(&vertices[0], vertices.size(), attrib.stride, &attrib, 1), shader, color};

    return line;
}

void RenderLine(Line *line)
{
    UseShader(line->shader);
    ShaderSetVec4(line->shader, "u_color", glm::vec4(line->color, 1.0f));
    ShaderSetMatrix4(line->shader, "u_model", glm::mat4(1.0f));

    glBindVertexArray(line->mesh.vao);
    glDrawArrays(GL_LINES, 0, line->mesh.verticesCount);

    glBindVertexArray(0);
}

void UpdateLine(Line *line, glm::vec3 start, glm::vec3 end)
{
    line->start = start;
    line->end = end;
    glm::vec3 vertices[2] = {start, end};
    UpdateMesh(&line->mesh, vertices, sizeof(vertices));
}