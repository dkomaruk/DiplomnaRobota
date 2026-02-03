#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

uniform mat4 u_viewProjInverse;

out vec3 EyeDir;

void main()
{
    vec4 screenPos = vec4((texCoords.x * 2.0) - 1.0, (texCoords.y * 2.0) - 1.0, 1.0, 1.0);
    vec4 worldPos = u_viewProjInverse * screenPos;
    EyeDir = worldPos.xyz / worldPos.w;

    gl_Position = vec4(pos.xy, 1.0, 1.0);
}