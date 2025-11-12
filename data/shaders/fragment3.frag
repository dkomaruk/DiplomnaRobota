#version 460 core

uniform sampler2D u_outline;
uniform sampler2D u_scene;

uniform float u_time;

in vec2 TexCoords;

void main()
{
    vec2 u_texelSize = vec2(1.0 / 1280.0, 1.0 / 720.0);
    vec3 outlineColor = vec3(1.0, 1.0, 1.0);
    int outlineThickness = 2;

    vec4 sceneColor = texture(u_scene, TexCoords);
    float currentPixel = texture(u_outline, TexCoords).r;

    if(currentPixel < 0.5)
    {
        gl_FragColor = sceneColor;
        return;
    }

    bool isEdge = false;
    for(int x = -outlineThickness; x <= outlineThickness; ++x)
    {
        for(int y = -outlineThickness; y <= outlineThickness; ++y)
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

    if(isEdge)
    {
        //float cycleLength = 1.0;
        //float onDuration = 0.5;

        //if(mod(u_time, cycleLength) < onDuration)
        //{
            //gl_FragColor = vec4(outlineColor, 1.0);
        //}
        //else
        //{
            //gl_FragColor = sceneColor;
        //}

        float pulseSpeed = 5.0;
        float sinWave = sin(u_time * pulseSpeed);
        //float pulse = (sinWave * 0.5) + 0.5;

        float minBrightness = 0.3;
        float pulse = (sinWave * (1.0 - minBrightness)) + minBrightness;
        gl_FragColor = mix(sceneColor, vec4(outlineColor, 1.0), pulse);
    }
    else
    {
        gl_FragColor = sceneColor;
    }
}