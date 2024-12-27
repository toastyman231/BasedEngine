#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;
in mat3 TBN;

#include "material.glsl"
#include "globals.glsl"
#include "lighting.glsl"

void main()
{
    vec3 normal = fragNormal;
    vec3 viewDir = normalize(vec3(eyePos) - fragPos);
    if (material.normalMat.useSampler == 1) {
        normal = vec3(GetNormalMaterial(uvs));
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN * normal);
    }

    if (renderMode == 0) {
        outColor = vec4(CalculateLighting(material, uvs, normal, fragPos, viewDir, fragPosLightSpace), 1.0);
    } else if (renderMode == 1) {
        outColor = GetDiffuseMaterial(uvs);
    } else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
}