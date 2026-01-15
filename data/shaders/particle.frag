#version 460 core

uniform sampler2D u_texture;

in vec2 TexCoords;
in vec4 Color;

void main()
{
    //if(Color.a < (1.0 / 255.0))
    //{
    //    discard;
    //}

    //vec4 color = vec4(2.0, 2.0, 2.0, Color.a);
    gl_FragColor = texture(u_texture, TexCoords) * Color;
}