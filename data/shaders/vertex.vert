#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal;

uniform mat4 u_model;
uniform mat3 u_normalMatrix;
uniform mat4 u_view;
uniform mat4 u_projection;

//out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    //TexCoords = texCoords;

    Normal = u_normalMatrix * normal;
    FragPos = vec3(u_model * vec4(pos, 1.0));

    gl_Position = u_projection * u_view * u_model * vec4(pos, 1.0);
}