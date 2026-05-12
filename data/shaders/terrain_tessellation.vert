#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;

out vec2 vs_TexCoords;
out vec3 vs_FragPos;

void main()
{
    vs_TexCoords = texCoords;
    vs_FragPos = pos;

    gl_Position = vec4(pos, 1.0);
}