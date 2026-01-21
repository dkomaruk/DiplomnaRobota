#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;

uniform mat4 u_projection;
uniform mat4 u_view;

out float Height;
out vec2 TexCoords;

void main()
{
    Height = pos.y;
    TexCoords = texCoords;

    gl_Position = u_projection * u_view * vec4(pos, 1.0);
}