#version 430 core

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

    /*if (renderMode == 0) {
        outColor = vertColor;//vec4(CalculateLighting(vec3(vertColor), uvs, normal, fragPos, viewDir, fragPosLightSpace), 1.0);
    } else if (renderMode == 1) {
        outColor = GetDiffuseMaterial(uvs);
    } else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
    }*/
    outColor = vec4(0, 1, 0, 1);//vertColor;
}