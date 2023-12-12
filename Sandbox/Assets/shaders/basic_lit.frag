#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;

#include "material.glsl"
#include "globals.glsl"
#include "lighting.glsl"

void main()
{
    vec3 viewDir = normalize(vec3(eyePos) - fragPos);
    outColor = vec4(CalculateLighting(material, uvs, fragNormal, fragPos, viewDir), 1.0);
}