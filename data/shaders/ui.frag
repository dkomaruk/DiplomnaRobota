#version 460 core

uniform sampler2D u_texture;

in vec2 TexCoords;

void main()
{
    gl_FragColor = texture(u_texture, TexCoords);
}