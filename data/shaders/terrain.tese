#version 460 core

//layout(quads) in;
layout(quads, fractional_odd_spacing) in;
//layout(quads, fractional_even_spacing) in;
//layout(quads, equal_spacing) in;

layout(binding = 0) uniform sampler2D u_heightmap;

in vec2 tsc_TexCoords[];

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_lightViewProj;

out vec2 TexCoords;
out vec4 FragPosLightSpace;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t0 = mix(tsc_TexCoords[0], tsc_TexCoords[1], u);
    vec2 t1 = mix(tsc_TexCoords[3], tsc_TexCoords[2], u);
    TexCoords = mix(t0, t1, v);

    vec4 pos0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, u);
    vec4 pos1 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, u);
    vec4 pos = mix(pos0, pos1, v);
    pos.y = texture(u_heightmap, TexCoords).r;

    FragPosLightSpace = u_lightViewProj * pos;

    gl_Position = u_projection * u_view * pos;
}