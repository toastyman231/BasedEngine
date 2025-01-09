#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;
in float heightPercent;
in vec4 fragPosLightSpace;

#include "material.glsl"
#include "globals.glsl"
#include "lighting.glsl"

void main()
{
    vec3 baseColor = vec3(0.05, 0.2, 0.01);
    vec3 tipColor = vec3(0.5, 0.5, 0.1);

    vec3 diffuseColor = mix(baseColor, tipColor, heightPercent);
    vec3 viewDir = normalize(vec3(eyePos) - fragPos);
    
    if (renderMode == 0) {
        outColor = vec4(CalculateLighting(material, uvs, fragNormal, fragPos, viewDir, fragPosLightSpace) * diffuseColor, 1.0);
    } else if (renderMode == 1) {
        outColor = vec4(diffuseColor, 1.0);
    } else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
}