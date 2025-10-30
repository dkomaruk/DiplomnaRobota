#version 460 core

uniform sampler2D u_texture;
uniform vec3 u_objectColor;
uniform vec3 u_viewPos;

uniform float u_time;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;

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

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

void main()
{
    vec3 normal = normalize(Normal);

    vec3 lightDir = normalize(u_light.position - FragPos);

    vec3 diffuseTexture = vec3(texture(u_material.diffuse, TexCoords));
    float diffuseStrength = max(0.0, dot(normal, lightDir));
    vec3 diffuse = (diffuseStrength * diffuseTexture) * u_light.diffuse;

    vec3 ambient = diffuseTexture * u_light.ambient;

    vec3 viewDir = normalize(u_viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specularTexture = vec3(texture(u_material.specular, TexCoords));
    vec3 specular = (specularStrength * specularTexture) * u_light.specular;

    //gl_FragColor = texture(u_texture, TexCoords);
    vec3 finalColor = diffuse + ambient + specular;

    vec3 emissionTexture = vec3(texture(u_material.emission, vec2(TexCoords.x, TexCoords.y + u_time / 2.0f)));
    finalColor += (1.0 - sign(specularTexture)) * emissionTexture;

    gl_FragColor = vec4(finalColor, 1.0);
}