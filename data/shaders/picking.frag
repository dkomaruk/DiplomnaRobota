#version 460 core

uniform uint u_objectIndex;

out vec3 FragColor;

void main()
{
    gl_FragColor = vec4(float(u_objectIndex) / 255.0, float(u_objectIndex) / 255.0, float(u_objectIndex) / 255.0, 1.0);
}