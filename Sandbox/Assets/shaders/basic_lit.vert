#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
layout (location = 2) in vec3 normal;

out vec2 uvs;
out vec3 fragNormal;
out vec3 fragPos;
out vec4 fragPosLightSpace;

#include "globals.glsl"
uniform mat4 model = mat4(1.0);
uniform mat4 normalMat = mat4(1.0);
uniform mat4 lightSpaceMatrix;

void main()
{
    uvs = texcoords;
    fragNormal = normalize(mat3(normalMat) * normal);
    fragPos = vec3(model * vec4(position, 1.0));
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    gl_Position = proj * view * model * vec4(position, 1.0);
}