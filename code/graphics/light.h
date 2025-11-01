#ifndef LIGHT_H

#include <glm/vec3.hpp>

using namespace glm;

struct DirectionalLight
{
    vec3 direction;
    vec3 diffuse, ambient, specular;
};

struct PointLight
{
    vec3 position;
    float constant, linear, quadratic;
    vec3 diffuse, ambient, specular;
};

struct SpotLight
{
    vec3 position, direction;
    float innerCutOff, outerCutOff;
    vec3 diffuse, ambient, specular;
};

DirectionalLight CreateDirLight(vec3 direction, vec3 diffuse = vec3(1.0f), vec3 ambient = vec3(1.0f), vec3 specular = vec3(1.0f));
PointLight CreatePointLight(vec3 position, float constant, float linear, float quadratic,
                            vec3 diffuse = vec3(1.0f), vec3 ambient = vec3(1.0f), vec3 specular = vec3(1.0f));
SpotLight CreateSpotLight(vec3 position, vec3 direction, float innerCutOff, float outerCutoff,
                          vec3 diffuse = vec3(1.0f), vec3 ambient = vec3(1.0f), vec3 specular = vec3(1.0f));


#define LIGHT_H
#endif