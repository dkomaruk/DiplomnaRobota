#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout(location = 3) in float scale;
layout(location = 4) in float angle;
layout(location = 5) in vec2 uvOffset;
layout(location = 6) in vec2 uvScale;
layout(location = 7) in vec3 offset;
layout(location = 8) in vec4 color;

uniform mat4 u_projection;
uniform mat4 u_view;

out vec2 TexCoords;
out vec4 Color;

void main()
{
    TexCoords = uvOffset + texCoords * uvScale;
    Color = color;

    vec3 cameraRight = vec3(u_view[0][0], u_view[1][0], u_view[2][0]);
    vec3 cameraUp = vec3(u_view[0][1], u_view[1][1], u_view[2][1]);

    float c = cos(angle);
    float s = sin(angle);

    vec2 rotatedPos = vec2(pos.x * c - pos.y * s, pos.x * s + pos.y * c);

    vec3 worldPos = offset + cameraRight * rotatedPos.x * scale + cameraUp * rotatedPos.y * scale;

    gl_Position = u_projection * u_view * vec4(worldPos, 1.0);
}