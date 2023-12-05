#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;

#include "material.glsl"
#include "globals.glsl"
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform float ambientStrength;

void main()
{
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(vec3(eyePos) - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininessMat.color.x);
    vec3 specular = lightColor * spec;

    vec4 result = vec4(ambient + diffuse + specular, 1.0) * GetDiffuseMaterial(uvs);
    outColor = result;
}