#version 410 core

out vec4 outColor;
in vec2 uvs;

#include "globals.glsl"
#include "pbr.glsl"

uniform sampler2D SceneColor;

void main()
{
    vec4 scene = texture(SceneColor, uvs);
    outColor = vec4(SRGBToLinear(vec3(0.157, 0.173, 0.204)), 1.0);//vec4(pow(scene.rgb, vec3(1.0/2.2)), scene.a); // TODO: Make gamma a UBO var
}