#version 460 core

layout(location = 0) in vec3 pos;

uniform mat4 u_model;
uniform mat4 u_lightViewProj;

void main()
{
    gl_Position = u_lightViewProj * u_model * vec4(pos, 1.0);
}