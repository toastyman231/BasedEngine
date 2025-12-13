#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

out vec3 fragNormal;

#include "globals.glsl"
uniform mat4 model = mat4(1.0);
uniform mat4 normalMat = mat4(1.0);

void main()
{
    gl_Position = view * model * vec4(position, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    fragNormal = normalize(vec3(vec4(normalMatrix * normal, 0.0)));
}