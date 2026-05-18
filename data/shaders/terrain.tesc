#version 460 core

layout(vertices = 4) out;

uniform float u_minDist = 2.0;
uniform float u_maxDist = 100.0;

uniform float u_minTessLevel = 1.0;
uniform float u_maxTessLevel = 32.0;

uniform mat4 u_view;

in vec2 vs_TexCoords[];

out vec2 tsc_TexCoords[];

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    tsc_TexCoords[gl_InvocationID] = vs_TexCoords[gl_InvocationID];

    vec4 vp00 = u_view * gl_in[0].gl_Position;
    vec4 vp01 = u_view * gl_in[1].gl_Position;
    vec4 vp10 = u_view * gl_in[2].gl_Position;
    vec4 vp11 = u_view * gl_in[3].gl_Position;

    float l00 = length(vp00.xyz);
    float l01 = length(vp01.xyz);
    float l10 = length(vp10.xyz);
    float l11 = length(vp11.xyz);

    float distBL = clamp((l00 - u_minDist) / (u_maxDist - u_minDist), 0.0, 1.0);
    float distBR = clamp((l01 - u_minDist) / (u_maxDist - u_minDist), 0.0, 1.0);
    float distTR = clamp((l10 - u_minDist) / (u_maxDist - u_minDist), 0.0, 1.0);
    float distTL = clamp((l11 - u_minDist) / (u_maxDist - u_minDist), 0.0, 1.0);

    float tessLevel0 = mix(u_maxTessLevel, u_minTessLevel, min(distTL, distBL));
    float tessLevel1 = mix(u_maxTessLevel, u_minTessLevel, min(distBL, distBR));
    float tessLevel2 = mix(u_maxTessLevel, u_minTessLevel, min(distBR, distTR));
    float tessLevel3 = mix(u_maxTessLevel, u_minTessLevel, min(distTL, distTR));

    gl_TessLevelOuter[0] = tessLevel0; //Outer Left
    gl_TessLevelOuter[1] = tessLevel1; //Outer Bottom
    gl_TessLevelOuter[2] = tessLevel2; //Outer Right
    gl_TessLevelOuter[3] = tessLevel3; //Outer Top

    gl_TessLevelInner[0] = max(tessLevel1, tessLevel3); //Inner TopBottom
    gl_TessLevelInner[1] = max(tessLevel0, tessLevel2); //Inner LeftRight
}