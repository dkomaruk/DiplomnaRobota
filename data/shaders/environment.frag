#version 460 core

in vec3 EyeDir;

uniform sampler2D u_skyMap;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

void main()
{
    vec3 dir = normalize(EyeDir);

    float orientation = PI;
    float phi = atan(dir.z, dir.x) + orientation;
    float theta = acos(dir.y);

    float u = phi / TWO_PI + 0.5;
    float v = theta / (PI * 0.5);
    vec2 uv = vec2(u, v);

    vec3 skyColor = texture(u_skyMap, uv).rgb;

    float horizonAngle = PI * 0.5;
    float horizonBlurRange = 0.03491 * 2.0;
    float blendStart = horizonAngle - horizonBlurRange;
    float blendFactor = smoothstep(blendStart, horizonAngle, theta);

    if(theta > horizonAngle)
    {
        blendFactor = 1.0;
    }

    vec3 groundColor = vec3(0.15789, 0.11663, 0.11663);
    //groundColor = vec3(0.4274, 0.9333, 1.0);
    groundColor = vec3(0.2509, 0.6078, 1.0);
    vec3 color = mix(skyColor * 2.5, groundColor, blendFactor);
    gl_FragColor = vec4(color, 1.0);
}