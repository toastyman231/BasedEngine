#version 410 core

out vec4 outColor;
in vec2 uvs;

#include "material.glsl"
#include "globals.glsl"

uniform sampler2D ShadowMap;
uniform sampler2D SceneColor;

void main()
{
    outColor = texture(SceneColor, uvs) * vec4(1.0, 0.0, 0.0, 1.0);
}