#version 460 core

layout(quads) in;

layout(binding = 0) uniform sampler2D u_heightmap;

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
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t0 = mix(tsc_TexCoords[0], tsc_TexCoords[1], u);
    vec2 t1 = mix(tsc_TexCoords[3], tsc_TexCoords[2], u);
    TexCoords = mix(t0, t1, v);

    vec3 n0 = mix(tsc_Normal[0], tsc_Normal[1], u);
    vec3 n1 = mix(tsc_Normal[3], tsc_Normal[2], u);
    Normal = normalize(mix(n0, n1, v));

    vec3 p0 = mix(tsc_FragPos[0], tsc_FragPos[1], u);
    vec3 p1 = mix(tsc_FragPos[3], tsc_FragPos[2], u);
    FragPos = mix(p0, p1, v);

    vec4 l0 = mix(tsc_FragPosLightSpace[0], tsc_FragPosLightSpace[1], u);
    vec4 l1 = mix(tsc_FragPosLightSpace[3], tsc_FragPosLightSpace[2], u);
    FragPosLightSpace = mix(l0, l1, v);

    float h0 = mix(tsc_Height[0], tsc_Height[1], u);
    float h1 = mix(tsc_Height[3], tsc_Height[2], u);
    Height = mix(h0, h1, v);

    vec4 pos0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, u);
    vec4 pos1 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, u);
    gl_Position = mix(pos0, pos1, v);
}