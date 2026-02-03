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
    vec2 uv = vec2(phi / TWO_PI + 0.5, theta / (PI * 0.5));

    vec3 skyColor = texture(u_skyMap, uv).rgb;

    float horizonAngle = PI * 0.5;
    float horizonBlurRange = 0.03491 * 2.0;
    float blendStart = horizonAngle - horizonBlurRange;
    float blendFactor = smoothstep(blendStart, horizonAngle, theta);

    vec3 groundColor = vec3(0.2509, 0.6078, 1.0);
    float intensity = 2.5;

    vec3 color = mix(skyColor * intensity, groundColor, blendFactor);
    gl_FragColor = vec4(color, 1.0);
}