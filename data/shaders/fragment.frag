#version 460 core

uniform sampler2D u_texture;

in vec2 TexCoords;
in vec3 Color;

void main()
{
    //gl_FragColor = texture(u_texture, TexCoords);
    gl_FragColor = vec4(Color, 1.0);
}