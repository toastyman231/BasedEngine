#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;

#include "material.glsl"
#include "globals.glsl"
#include "lighting.glsl"
uniform vec3 cratePos;

void main()
{
    float dist = distance(cratePos, fragPos) / 5.0;
    vec3 diffuseColor = vec3(1 - dist, 0.0, dist);

    vec3 viewDir = normalize(vec3(eyePos) - fragPos);
    outColor = vec4(CalculateLighting(material, uvs, fragNormal, fragPos, viewDir) * diffuseColor, 1.0);
}