#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 texCoords;

uniform mat4 u_model;
uniform mat4 u_lightViewProj;

out vec2 TexCoords;

void main()
{
    TexCoords = texCoords;
    gl_Position = u_lightViewProj * u_model * vec4(pos, 1.0);
}