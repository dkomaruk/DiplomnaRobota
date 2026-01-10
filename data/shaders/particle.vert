#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec4 color;
layout(location = 4) in vec3 offset;
//layout(location = 4) in mat4 model;

//uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

out vec2 TexCoords;
out vec4 Color;

void main()
{
    TexCoords = texCoords;
    Color = color;

    vec3 cameraUp = vec3(u_view[0][0], u_view[1][0], u_view[2][0]);
    vec3 cameraRight = vec3(u_view[0][1], u_view[1][1], u_view[2][1]);
    float size = 0.4;
    vec3 worldPos = offset + cameraRight * pos.x * size + cameraUp * pos.y * size;

    //vec3 scale = vec3(1.0, 1.0, 1.0);
    //gl_Position = u_projection * u_view * vec4((pos * scale) + offset, 1.0);

    //gl_Position = u_projection * model * vec4(pos, 1.0);

    //gl_Position = u_projection * u_view * vec4(pos * 0.2 + offset, 1.0);

    gl_Position = u_projection * u_view * vec4(worldPos, 1.0);
}