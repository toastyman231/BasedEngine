#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in invec4 boneIds;
layout (location = 6) in vec4 weights;

out vec2 uvs;
out vec3 fragNormal;
out vec3 fragPos;
out vec4 fragPosLightSpace;
out mat3 TBN;

#include "material.glsl"
#include "globals.glsl"
uniform mat4 model = mat4(1.0);
uniform mat4 normalMat = mat4(1.0);
uniform mat4 lightSpaceMatrix;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    uvs = texcoords;
    fragNormal = normalize(mat3(normalMat) * normal);
    fragPos = vec3(model * vec4(position, 1.0));
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * fragNormal;
    }

    gl_Position = proj * view * model * vec4(position, 1.0);
}