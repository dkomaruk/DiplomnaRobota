#version 460 core

layout(vertices = 4) out;

in float vs_Height[];
in vec2 vs_TexCoords[];
in vec3 vs_Normal[];
in vec3 vs_FragPos[];
in vec4 vs_FragPosLightSpace[];

out float tsc_Height[];
out vec2 tsc_TexCoords[];
out vec3 tsc_Normal[];
out vec3 tsc_FragPos[];
out vec4 tsc_FragPosLightSpace[];

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    tsc_Height[gl_InvocationID] = vs_Height[gl_InvocationID];
    tsc_TexCoords[gl_InvocationID] = vs_TexCoords[gl_InvocationID];
    tsc_Normal[gl_InvocationID] = vs_Normal[gl_InvocationID];
    tsc_FragPos[gl_InvocationID] = vs_FragPos[gl_InvocationID];
    tsc_FragPosLightSpace[gl_InvocationID] = vs_FragPosLightSpace[gl_InvocationID];

    gl_TessLevelOuter[0] = 2.0; //Outer Left
    gl_TessLevelOuter[1] = 2.0; //Outer Bottom
    gl_TessLevelOuter[2] = 2.0; //Outer Right
    gl_TessLevelOuter[3] = 2.0; //Outer Top

    gl_TessLevelInner[0] = 2.0; //Inner TopBottom
    gl_TessLevelInner[1] = 2.0; //Inner LeftRight
}