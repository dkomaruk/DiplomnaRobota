#version 460 core

uniform sampler2D u_terrainMap;

uniform sampler2D u_splatMap;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;
uniform sampler2D u_texture3;

in float Height;
in vec2 TexCoords;

void main()
{
    vec4 weights = texture(u_splatMap, TexCoords);

    vec2 tiledUV = TexCoords * 16.0;
    vec4 texture0 = texture(u_texture0, tiledUV);
    vec4 texture1 = texture(u_texture1, tiledUV);
    vec4 texture2 = texture(u_texture2, tiledUV);
    vec4 texture3 = texture(u_texture3, tiledUV);

    float baseWeight = max(0.0, 1.0 - (weights.r + weights.g + weights.b));

    gl_FragColor = (texture0 * baseWeight) + (texture1 * weights.r) + (texture2 * weights.g) + (texture3 * weights.b);
}