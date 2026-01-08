#version 460 core

uniform sampler2D u_texture;
uniform vec3 u_textColor;

in vec2 TexCoords;

void main()
{
    gl_FragColor = texture(u_texture, TexCoords) * vec4(u_textColor, 1.0);
}