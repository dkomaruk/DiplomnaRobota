#include "light.h"

DirectionalLight CreateDirLight(vec3 direction, vec3 diffuse, vec3 ambient, vec3 specular)
{
    DirectionalLight light = {direction, diffuse, ambient, specular};
    return light;
}

PointLight CreatePointLight(vec3 position, float constant, float linear, float quadratic, vec3 diffuse, vec3 ambient, vec3 specular)
{
    PointLight light = {position, constant, linear, quadratic, diffuse, ambient, specular};
    return light;
}

SpotLight CreateSpotLight(vec3 position, vec3 direction, float innerCutOff, float outerCutOff, vec3 diffuse, vec3 ambient, vec3 specular)
{
    SpotLight light = {position, direction, innerCutOff, outerCutOff, diffuse, ambient, specular};
    return light;
}