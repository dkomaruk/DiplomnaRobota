#version 460 core

uniform sampler2D u_outline;
uniform sampler2D u_scene;
uniform sampler2D u_smoke;
uniform sampler2D u_sceneDepth;
uniform sampler2D u_smokeDepth;
uniform vec2 u_lowResInvSize;

uniform int u_outlineThickness;

uniform bool u_inverted;
uniform bool u_grayscale;
uniform bool u_showOutline;

uniform float u_time;

in vec2 TexCoords;

const float offset = 1.0 / 300.0;

void main()
{
    vec2 u_texelSize = vec2(1.0 / 1280.0, 1.0 / 720.0);
    vec3 outlineColor = vec3(1.0, 1.0, 1.0);
    vec4 sceneColor = texture(u_scene, TexCoords);

    vec4 smokeColor = texture(u_smoke, TexCoords);
    sceneColor = vec4(mix(sceneColor.rgb, smokeColor.rgb, smokeColor.a), 1.0);

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

    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right
    );

    float kernel0[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    float kernel1[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );

    float kernel2[9] = float[](
        1.0 / 9, 1.0 / 9, 1.0 / 9,
        1.0 / 9, 1.0 / 9, 1.0 / 9,
        1.0 / 9, 1.0 / 9, 1.0 / 9
    );

    float kernel3[9] = float[](
        0, -1, 0,
        -1,  5, -1,
        0, -1, 0
    );

    float kernel4[9] = float[](
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1
    );

    float kernel5[9] = float[](
        0, -1, 0,
        -1,  4, -1,
        0, -1, 0
    );

    float kernel[9] = float[](
        2, 2, 2,
        2, -16, 2,
        2, 2, 2
    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(u_scene, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    //sceneColor = vec4(col, 1.0);

    bool isEdge = false;
    if(u_showOutline)
    {
        float currentPixel = texture(u_outline, TexCoords).r;

        if(currentPixel < 0.5)
        {
            gl_FragColor = sceneColor;
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
        gl_FragColor = mix(sceneColor, vec4(outlineColor, 1.0), pulse);
    }
    else
    {
        gl_FragColor = sceneColor;
    }
}