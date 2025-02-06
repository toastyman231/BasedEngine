#version 410 core

out vec4 outColor;
in vec2 uvs;

#include "material.glsl"
#include "globals.glsl"

uniform vec4 color;

void main()
{
    outColor = color;
}