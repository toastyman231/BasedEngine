#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;

#include "material.glsl"
#include "globals.glsl"

void main()
{
    outColor = GetDiffuseMaterial(uvs);
}