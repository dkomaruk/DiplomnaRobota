#ifndef LIGHT_H

#include <glm/vec3.hpp>

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 diffuse, ambient, specular;
};

struct PointLight
{
    glm::vec3 position;
    float constant, linear, quadratic;
    glm::vec3 diffuse, ambient, specular;
};

struct SpotLight
{
    glm::vec3 position, direction;
    float innerCutOff, outerCutOff;
    glm::vec3 diffuse, ambient, specular;
};

DirectionalLight CreateDirLight(glm::vec3 direction, glm::vec3 diffuse = glm::vec3(1.0f), glm::vec3 ambient = glm::vec3(1.0f), glm::vec3 specular = glm::vec3(1.0f));
PointLight CreatePointLight(glm::vec3 position, float constant, float linear, float quadratic,
                            glm::vec3 diffuse = glm::vec3(1.0f), glm::vec3 ambient = glm::vec3(1.0f), glm::vec3 specular = glm::vec3(1.0f));
SpotLight CreateSpotLight(glm::vec3 position, glm::vec3 direction, float innerCutOff, float outerCutoff,
                          glm::vec3 diffuse = glm::vec3(1.0f), glm::vec3 ambient = glm::vec3(1.0f), glm::vec3 specular = glm::vec3(1.0f));


#define LIGHT_H
#endif