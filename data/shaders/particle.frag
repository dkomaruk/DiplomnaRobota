#version 460 core

uniform sampler2D u_texture;
uniform sampler2D u_atlas;

uniform sampler2D u_sceneDepth;
uniform vec2 u_screenSize;

in vec2 TexCoords;
in vec4 Color;

void main()
{
    //if(Color.a < (1.0 / 255.0))
    //{
    //    discard;
    //}

    //vec4 color = vec4(2.0, 2.0, 2.0, Color.a);
    //gl_FragColor = texture(u_texture, TexCoords) * Color;

    vec2 screenUV = gl_FragCoord.xy / u_screenSize;
    float sceneDepth = texture(u_sceneDepth, screenUV).r;

    if(gl_FragCoord.z > sceneDepth)
    {
        discard;
    }

    vec4 color = texture(u_texture, TexCoords);
    gl_FragColor = color * Color;

    //gl_FragColor = texture(u_atlas, TexCoords) * Color;
}