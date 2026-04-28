#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 normal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_lightViewProj;

out float Height;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

//out float vs_Height;
//out vec2 vs_TexCoords;
//out vec3 vs_Normal;
//out vec3 vs_FragPos;
//out vec4 vs_FragPosLightSpace;

void main()
{
    Height = pos.y;
    TexCoords = texCoords;
    Normal = normal;
    FragPos = pos;
    FragPosLightSpace = u_lightViewProj * vec4(FragPos, 1.0);

    //vs_Height = pos.y;
    //vs_TexCoords = texCoords;
    //vs_Normal = normal;
    //vs_FragPos = pos;
    //vs_FragPosLightSpace = u_lightViewProj * vec4(vs_FragPos, 1.0);

    gl_Position = u_projection * u_view * vec4(pos, 1.0);
}