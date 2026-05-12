#version 460 core

layout(vertices = 4) out;

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

    float minDist = 2.0;
    float maxDist = 50.0;

    float dist00 = clamp((l00 - minDist) / (maxDist - minDist), 0.0, 1.0);
    float dist01 = clamp((l01 - minDist) / (maxDist - minDist), 0.0, 1.0);
    float dist10 = clamp((l10 - minDist) / (maxDist - minDist), 0.0, 1.0);
    float dist11 = clamp((l11 - minDist) / (maxDist - minDist), 0.0, 1.0);

    float minTessLevel = 1;
    float maxTessLevel = 8;

    float tessLevel0 = mix(maxTessLevel, minTessLevel, min(dist10, dist00));
    float tessLevel1 = mix(maxTessLevel, minTessLevel, min(dist00, dist01));
    float tessLevel2 = mix(maxTessLevel, minTessLevel, min(dist01, dist11));
    float tessLevel3 = mix(maxTessLevel, minTessLevel, min(dist11, dist10));

    gl_TessLevelOuter[0] = tessLevel0; //Outer Left
    gl_TessLevelOuter[1] = tessLevel1; //Outer Bottom
    gl_TessLevelOuter[2] = tessLevel2; //Outer Right
    gl_TessLevelOuter[3] = tessLevel3; //Outer Top

    gl_TessLevelInner[0] = max(tessLevel1, tessLevel3); //Inner TopBottom
    gl_TessLevelInner[1] = max(tessLevel0, tessLevel2); //Inner LeftRight
}