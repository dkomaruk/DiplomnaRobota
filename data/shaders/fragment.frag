#version 460 core

uniform sampler2D u_texture;
uniform sampler2D u_texture2;
uniform float u_mix;

in vec2 TexCoords;

void main()
{
    //gl_FragColor = texture(u_texture, TexCoords) * vec4(Col, 1.0) * 2;
    gl_FragColor = mix(texture(u_texture, TexCoords), texture(u_texture2, vec2(-TexCoords.x, TexCoords.y)), u_mix);
}