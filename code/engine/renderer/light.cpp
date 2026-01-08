#include "light.h"

DirectionalLight CreateDirLight(glm::vec3 direction, glm::vec3 diffuse, glm::vec3 ambient, glm::vec3 specular)
{
    DirectionalLight light = {direction, diffuse, ambient, specular};
    return light;
}

PointLight CreatePointLight(glm::vec3 position, float constant, float linear, float quadratic, glm::vec3 diffuse, glm::vec3 ambient, glm::vec3 specular)
{
    PointLight light = {position, constant, linear, quadratic, diffuse, ambient, specular};
    return light;
}

SpotLight CreateSpotLight(glm::vec3 position, glm::vec3 direction, float innerCutOff, float outerCutOff, glm::vec3 diffuse, glm::vec3 ambient, glm::vec3 specular)
{
    SpotLight light = {position, direction, innerCutOff, outerCutOff, diffuse, ambient, specular};
    return light;
}