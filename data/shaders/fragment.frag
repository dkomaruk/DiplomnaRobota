#version 460 core

uniform sampler2D u_texture;

uniform vec3 u_objectColor;

uniform vec3 u_viewPos;

struct Material
{
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;

    float shininess;
};

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material u_material;
uniform Light u_light;

in vec3 Normal;
in vec3 FragPos;

void main()
{
    vec3 normal = normalize(Normal);

    vec3 ambient = u_material.ambient * u_light.ambient;

    vec3 lightDir = normalize(u_light.position - FragPos);

    float diffuseStrength = max(0.0, dot(normal, lightDir));
    vec3 diffuse = (diffuseStrength * u_material.diffuse) * u_light.diffuse;

    vec3 viewDir = normalize(u_viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specular = (specularStrength * u_material.specular) * u_light.specular;

    //gl_FragColor = texture(u_texture, TexCoords);
    vec3 finalColor = diffuse + ambient + specular;
    gl_FragColor = vec4(finalColor, 1.0);
}