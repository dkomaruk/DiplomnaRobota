#version 460 core

uniform vec3 u_objectColor;
uniform vec2 u_viewport;

uniform vec3 u_viewPos;
uniform vec3 u_viewDir;

uniform float u_time;

uniform sampler2D u_outlineTexture;
uniform bool u_outlinePass;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;

    float shininess;
};

struct SampledTextures
{
    vec4 diffuse;
    vec4 specular;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient, diffuse, specular;
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

uniform Material u_material;

uniform int u_dirLightCount;
uniform DirLight u_dirLight;

#define MAX_POINT_LIGHTS 16
uniform int u_pointLightCount;
uniform PointLight u_pointLights[16];

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
//in vec3 DebugColor;

vec3 CalculateDirLight(DirLight light, SampledTextures textures, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diffuseStrength = max(0.0, dot(normal, lightDir));
    vec3 diffuse = diffuseStrength * textures.diffuse.rgb * light.diffuse;

    vec3 ambient = textures.diffuse.rgb * light.ambient;

    vec3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * textures.specular.rgb * light.specular;

    return diffuse + ambient + specular;
}

vec3 CalculatePointLight(PointLight light, SampledTextures textures, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diffuseStrength = max(0.0, dot(lightDir, normal));
    vec3 diffuse = textures.diffuse.rgb * diffuseStrength * light.diffuse;

    vec3 ambient = textures.diffuse.rgb * light.ambient;

    vec3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
    vec3 specular = specularStrength * textures.specular.rgb * light.specular;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear + light.quadratic * (distance * distance));

    return (diffuse + ambient + specular) * attenuation;
}

void main()
{
    SampledTextures sampledTextures;
    sampledTextures.diffuse = texture(u_material.diffuse, TexCoords);
    sampledTextures.specular = texture(u_material.specular, TexCoords);

    if(sampledTextures.diffuse.a < 0.1)
    {
        discard;
    }

    vec3 normal = Normal;
    if(!gl_FrontFacing)
    {
        normal = -normal;
    }

    vec3 viewDir = normalize(u_viewPos - FragPos);

    vec3 finalColor = vec3(0.0);
    finalColor += CalculateDirLight(u_dirLight, sampledTextures, normal, viewDir) * clamp(u_dirLightCount, 0, 1);
    for(int i = 0; i < u_pointLightCount; i++)
    {
        finalColor += CalculatePointLight(u_pointLights[i], sampledTextures, normal, FragPos, viewDir);
    }

    //finalColor = DebugColor;
    gl_FragColor = vec4(finalColor, 1.0);
}