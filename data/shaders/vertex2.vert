#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in mat4 model;
layout(location = 2) in vec2 texCoords;

uniform float u_time;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 TexCoords;
out vec3 Color;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec3 position = pos;
    TexCoords = texCoords;

    int gridSize = 170;
    int row = gl_InstanceID / gridSize;
    int col = gl_InstanceID % gridSize;

    vec2 seed = vec2(row, col) + u_time * 0.1;
    Color = vec3(rand(seed * 5.0), rand(seed + 2.0), rand(seed / 2.0));
    //Color = vec3(0.0, 0.0, rand(seed / 2.0));

    gl_Position = u_projection * u_view * model * vec4(position, 1.0);
}