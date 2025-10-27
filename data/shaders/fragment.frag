#version 460 core

uniform sampler2D u_texture;

uniform vec3 u_objectColor;
uniform vec3 u_lightColor;

in vec2 TexCoords;

void main()
{
    //gl_FragColor = texture(u_texture, TexCoords);
    gl_FragColor = vec4(u_objectColor * u_lightColor, 1.0);
}