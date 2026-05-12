#version 460 core

layout(binding = 0) uniform sampler2D u_heightmap;
layout(binding = 1) uniform sampler2D u_color;
layout(binding = 2) uniform sampler2D u_noiseMap;
layout(binding = 3) uniform sampler2D u_shadowMap;

uniform float u_texCoordsMultiplier;
uniform float u_mapScale;

struct DirLight
{
    vec3 direction;
    vec3 ambient, diffuse, specular;
};
uniform DirLight u_dirLight;

in vec2 TexCoords;
in vec4 FragPosLightSpace;

out vec4 FragColor;

//https://web.archive.org/web/20190211214453/https://www.iquilezles.org/www/articles/texturerepetition/texturerepetition.htm
//https://www.shadertoy.com/view/Xtl3zf
//Texture bombing to help with texture tiling pattern

float sum(vec3 v) { return v.x + v.y + v.z; }

vec2 hash_offsets(float i) {
    return fract(sin(vec2(i, i + 1.0)) * vec2(43758.5453, 12345.6789));
}

vec4 textureNoTile(sampler2D samp, sampler2D noise, in vec2 uv) {
    float k = texture(noise, uv * 0.0025).x;

    float index = k * 8.0;
    float i = floor(index);
    float f = fract(index);

    vec2 offa = hash_offsets(i);
    vec2 offb = hash_offsets(i + 1.0);

    vec2 dx = dFdx(uv);
    vec2 dy = dFdy(uv);

    vec4 cola = textureGrad(samp, uv + offa, dx, dy);
    vec4 colb = textureGrad(samp, uv + offb, dx, dy);

    return mix(cola, colb, smoothstep(0.2, 0.8, f - 0.1 * sum(cola.rgb - colb.rgb)));
}

float CalculateShadow(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(u_shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    vec2 tiledUV = TexCoords * u_texCoordsMultiplier;

    vec3 color = textureNoTile(u_color, u_noiseMap, tiledUV).rgb * 2.0;

    float left = textureOffset(u_heightmap, TexCoords, ivec2(-1, 0)).r;
    float right = textureOffset(u_heightmap, TexCoords, ivec2(1, 0)).r;
    float up = textureOffset(u_heightmap, TexCoords, ivec2(0, 1)).r;
    float down = textureOffset(u_heightmap, TexCoords, ivec2(0, -1)).r;

    vec3 normal = normalize(vec3(down - up, 2.0 * u_mapScale, left - right));

    vec3 lightDir = normalize(-u_dirLight.direction);

    float diffuseStrength = max(0.0, dot(normal, lightDir));
    vec3 diffuse = diffuseStrength * color * u_dirLight.diffuse;

    vec3 ambient = color * u_dirLight.ambient;

    float shadow = CalculateShadow(FragPosLightSpace);

    FragColor = vec4((ambient + (1.0 - shadow) * diffuse), 1.0);
}