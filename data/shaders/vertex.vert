#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;

uniform mat4 u_model;
uniform mat4 u_projection;

out vec2 TexCoords;

void main()
{
    vec3 position = pos;
    TexCoords = texCoords;
    gl_Position = u_projection * u_model * vec4(position, 1.0);
}