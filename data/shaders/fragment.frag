#version 460 core

uniform sampler2D u_texture;

uniform vec3 u_objectColor;

uniform vec3 u_viewPos;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;

in vec3 Normal;
in vec3 FragPos;

void main()
{
    vec3 normal = normalize(Normal);

    float ambientStrength = 0.1;
    vec3 ambientLight = u_lightColor * ambientStrength;

    vec3 lightDir = normalize(u_lightPos - FragPos);

    float diffuseStrength = max(0.0, dot(normal, lightDir));
    vec3 diffuseLight = diffuseStrength * u_lightColor;

    float specularStrength = 0.3;
    vec3 viewDir = normalize(u_viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specularLight = specularStrength * specular * u_lightColor;

    //gl_FragColor = texture(u_texture, TexCoords);
    vec3 finalColor = ((diffuseLight + ambientLight) *  u_objectColor) + specularLight;
    gl_FragColor = vec4(finalColor, 1.0);
}