#version 410 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec2 uvs[];
in vec3 fragNormal[];
in vec3 fragPos[];
in vec4 fragPosLightSpace[];
in mat3 TBN[];

const float MAGNITUDE = 0.1;

#include "globals.glsl"

void GenerateLine(int index)
{
    gl_Position = proj * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = proj * (gl_in[index].gl_Position + vec4(fragNormal[index], 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}  