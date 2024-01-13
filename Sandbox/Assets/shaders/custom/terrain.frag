#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;

#include "material.glsl"
#include "globals.glsl"
#include "lighting.glsl"

void main()
{
    vec3 groundColor = vec3(0.02, 0.52, 0.0);
    vec3 dirtColor = vec3(0.27, 0.18, 0.0);

    float mixFactor = dot(fragNormal, vec3(1, 0, 0));
    vec3 diffuseColor = mix(dirtColor, groundColor, mixFactor);

    vec3 viewDir = normalize(vec3(eyePos) - fragPos);
    outColor = vec4(CalculateLighting(material, uvs, fragNormal, fragPos, viewDir, fragPosLightSpace) * diffuseColor, 1.0);
}