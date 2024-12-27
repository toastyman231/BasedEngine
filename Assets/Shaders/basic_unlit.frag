#version 410 core

out vec4 outColor;
in vec2 uvs;

#include "material.glsl"
#include "globals.glsl"

void main()
{
    outColor = GetDiffuseMaterial(uvs);
}