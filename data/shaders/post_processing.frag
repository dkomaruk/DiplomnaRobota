#version 460 core

uniform sampler2D u_outline;
uniform sampler2D u_scene;
uniform sampler2D u_particles;
uniform sampler2D u_sceneDepth;
uniform sampler2D u_smokeDepth;
uniform vec2 u_lowResInvSize;

uniform int u_outlineThickness;

uniform bool u_inverted;
uniform bool u_grayscale;
uniform bool u_showOutline;
uniform bool u_showParticles;

uniform float u_time;

in vec2 TexCoords;

const float offset = 1.0 / 300.0;

out vec4 FragColor;

void main()
{
    vec2 u_texelSize = vec2(1.0 / 1280.0, 1.0 / 720.0);
    vec3 outlineColor = vec3(1.0, 1.0, 1.0);
    vec4 sceneColor = texture(u_scene, TexCoords);

    if(u_showParticles)
    {
        vec4 smokeColor = texture(u_particles, TexCoords);
        sceneColor = vec4(mix(sceneColor.rgb, smokeColor.rgb, smokeColor.a), 1.0);
    }

    if(u_inverted)
    {
        sceneColor = 1.0 - sceneColor;
        outlineColor = 1.0 - outlineColor;
    }
    if(u_grayscale)
    {
        float avg = sceneColor.r * 0.2126 + sceneColor.g * 0.7152 + sceneColor.b * 0.0722;
        sceneColor = vec4(avg, avg, avg, 1.0);
    }

    bool isEdge = false;
    if(u_showOutline)
    {
        float currentPixel = texture(u_outline, TexCoords).r;

        if(currentPixel < 0.5)
        {
            FragColor = sceneColor;
            return;
        }

        for(int x = -u_outlineThickness; x <= u_outlineThickness; ++x)
        {
            for(int y = -u_outlineThickness; y <= u_outlineThickness; ++y)
            {
                if(x == 0 && y == 0) continue;

                vec2 sampleOffset = vec2(float(x), float(y)) * u_texelSize;
                float neighbor = texture(u_outline, TexCoords + sampleOffset).r;

                if(neighbor < 0.5)
                {
                    isEdge = true;
                    break;
                }
            }
            if(isEdge) break;
        }
    }

    if(isEdge)
    {
        float pulseSpeed = 7.5;
        float sinWave = sin(u_time * pulseSpeed);
        //float pulse = (sinWave * 0.5) + 0.5;

        float minBrightness = 0.5;
        float pulse = (sinWave * (1.0 - minBrightness)) + minBrightness;
        FragColor = mix(sceneColor, vec4(outlineColor, 1.0), pulse);
    }
    else
    {
        FragColor = sceneColor;
    }
}