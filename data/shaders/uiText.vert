#version 460 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texCoords;

uniform mat4 u_model;
uniform mat4 u_projection;

out vec2 TexCoords;

void main()
{
    TexCoords = texCoords;
    gl_Position = u_projection * u_model * vec4(pos, 0.0, 1.0);
}