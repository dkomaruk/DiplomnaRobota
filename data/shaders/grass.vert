#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout(location = 3) in float angle;
layout(location = 4) in vec3 offset;

uniform mat4 u_projection;
uniform mat4 u_view;

out vec2 TexCoords;
out vec4 Color;

void main()
{
    TexCoords = texCoords;

    float c = cos(angle);
    float s = sin(angle);
    vec3 rotatedPos = vec3(pos.x * c + pos.y * s, pos.y, -pos.x * s + pos.z * c);

    //vec2 rotatedPos = pos.xy;

    //vec3 direction = vec3(0.0, 1.0, 0.0);

    //mat4 inverseView = inverse(u_view);
    //vec3 cameraPos = vec3(inverseView[3]);
    //vec3 directionToCamera = normalize(cameraPos - offset);

    //vec3 right = normalize(cross(direction, directionToCamera));

    //vec3 worldPos = offset + (right * rotatedPos.x) + (direction * rotatedPos.y);

    gl_Position = u_projection * u_view * vec4(offset + rotatedPos * 0.3, 1.0);
}