#version 460 core

uniform sampler2D u_texture;

in vec2 TexCoords;

void main()
{
    vec4 color = texture(u_texture, TexCoords);
    if(color.a < 0.4)
    {
        discard;
    }

    gl_FragColor = color;
}