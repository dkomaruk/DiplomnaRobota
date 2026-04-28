#version 460 core

layout(quads) in;

in float tsc_Height[];
in vec2 tsc_TexCoords[];
in vec3 tsc_Normal[];
in vec3 tsc_FragPos[];
in vec4 tsc_FragPosLightSpace[];

out float Height;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

void main()
{
    Height = tsc_Height[0];
    TexCoords = tsc_TexCoords[0];
    Normal = tsc_Normal[0];
    FragPos = tsc_FragPos[0];
    FragPosLightSpace = tsc_FragPosLightSpace[0];

    gl_Position = gl_in[0].gl_Position;
}