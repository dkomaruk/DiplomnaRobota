#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal;

uniform mat4 u_projection;
uniform mat4 u_view;

out float Height;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    Height = pos.y;
    TexCoords = texCoords;
    Normal = normal;
    FragPos = pos;

    gl_Position = u_projection * u_view * vec4(pos, 1.0);
}