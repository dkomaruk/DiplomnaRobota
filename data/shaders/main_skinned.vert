#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in ivec4 boneId;
layout(location = 4) in vec4 bnWeight;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

const int MAX_BONES = 206;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 u_skinning[MAX_BONES];

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
//out vec3 DebugColor;

void main()
{
    vec4 blendedPos = vec4(0.0);
    vec4 blendedNormal = vec4(0.0);

    for(int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if(boneId[i] == -1) continue;

        if(boneId[i] >= MAX_BONES)
        {
            blendedPos = vec4(position, 1.0);
            blendedNormal = vec4(normal, 0.0);
            break;
        }

        mat4 boneTransform = u_skinning[boneId[i]];

        blendedPos += boneTransform * vec4(position, 1.0) * bnWeight[i];
        blendedNormal += boneTransform * vec4(normal, 0.0) * bnWeight[i];
    }

    //blendedPos = vec4(position, 1.0); //Render in T-Pose

    TexCoords = texCoords;
    Normal = normalize((u_model * blendedNormal).xyz);
    FragPos = vec3(u_model * blendedPos);
    //DebugColor = bnWeight.xyz;
    gl_Position = u_projection * u_view * u_model * vec4(blendedPos.xyz, 1.0);
}