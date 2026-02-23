#version 460 core

uniform sampler2D u_terrainMap;

uniform sampler2D u_splatMap;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;
uniform sampler2D u_texture3;

uniform sampler2D u_noiseMap;

struct DirLight
{
    vec3 direction;
    vec3 ambient, diffuse, specular;
};
uniform DirLight u_dirLight;

in float Height;
in vec2 TexCoords;
in vec3 Normal;

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

void main()
{
    //vec4 weights = texture(u_splatMap, TexCoords);

    vec2 tiledUV = TexCoords * 16.0;
    //vec4 texture1 = texture(u_texture1, tiledUV);
    //vec4 texture0 = texture(u_texture0, tiledUV);
    //vec4 texture2 = texture(u_texture2, tiledUV);
    //vec4 texture3 = texture(u_texture3, tiledUV);
    //float baseWeight = max(0.0, 1.0 - (weights.r + weights.g + weights.b));

    vec3 texture1 = textureNoTile(u_texture1, u_noiseMap, tiledUV).rgb * 2.0;

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-u_dirLight.direction);

    float diffuseStrength = max(0.0, dot(normal, lightDir));
    vec3 diffuse = diffuseStrength * texture1 * u_dirLight.diffuse;

    vec3 ambient = texture1 * u_dirLight.ambient;

    gl_FragColor = vec4(diffuse + ambient, 1.0);
}