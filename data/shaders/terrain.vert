#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_lightViewProj;

layout(binding = 0) uniform sampler2D u_heightmap;
layout(binding = 1) uniform sampler2D u_normalmap;

out float Height;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

void main()
{
    vec3 position = pos;
    position.y = texture(u_heightmap, texCoords).r;

    Height = position.y;
    TexCoords = texCoords;
    //Normal = normal;
    Normal = texture(u_normalmap, texCoords).xyz;
    FragPos = position;
    FragPosLightSpace = u_lightViewProj * vec4(FragPos, 1.0);

    gl_Position = u_projection * u_view * vec4(position, 1.0);
}