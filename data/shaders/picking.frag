#version 460 core

uniform uint u_objectIndex;

out vec3 FragColor;

void main()
{
    FragColor = vec3(float(u_objectIndex) / 255.0, 0.0, 0.0);
}