#version 460 core

uniform sampler2D u_terrainMap;

in float Height;
in vec2 TexCoords;

void main()
{
    //float h = (Height + 6) / 6.0f;
    //gl_FragColor = vec4(h, h, h, 1.0);
    gl_FragColor = texture(u_terrainMap, TexCoords);
}