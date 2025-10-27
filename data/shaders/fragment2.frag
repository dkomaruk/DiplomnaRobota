#version 460 core

uniform vec3 u_lightColor;

void main()
{
    gl_FragColor = vec4(u_lightColor, 1.0);
}