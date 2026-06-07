#version 460 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;

    float shininess;
};

uniform Material u_material;

in vec2 TexCoords;

void main()
{
    if(texture(u_material.diffuse, TexCoords).a < 0.4)
    {
        discard;
    }
}