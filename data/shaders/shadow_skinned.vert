#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 3) in ivec4 boneId;
layout(location = 4) in vec4 bnWeight;

uniform mat4 u_model;
uniform mat4 u_lightViewProj;

const int MAX_BONES = 206;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 u_skinning[MAX_BONES];

void main()
{
    vec4 blendedPos = vec4(0.0);

    for(int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if(boneId[i] == -1) continue;

        if(boneId[i] >= MAX_BONES)
        {
            blendedPos = vec4(pos, 1.0);
            break;
        }

        mat4 boneTransform = u_skinning[boneId[i]];

        blendedPos += boneTransform * vec4(pos, 1.0) * bnWeight[i];
    }

    gl_Position = u_lightViewProj * u_model * vec4(blendedPos.xyz, 1.0);
}