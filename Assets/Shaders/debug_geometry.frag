#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;
in vec4 vertColor;

#include "material.glsl"
#include "globals.glsl"
#include "lighting.glsl"

void main()
{
    vec3 normal = fragNormal;
    vec3 viewDir = normalize(vec3(eyePos) - fragPos);

    outColor = vec4(0, 1, 0, 1);
}