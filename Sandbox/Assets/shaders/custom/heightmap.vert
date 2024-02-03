#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec2 uvs;
out vec3 fragNormal;
out vec3 fragPos;
out vec4 fragPosLightSpace;

uniform sampler2D heightmap;

#include "globals.glsl"
uniform mat4 model = mat4(1.0);
uniform float heightCoef = 1.0;
uniform mat4 normalMat = mat4(1.0);
uniform mat4 lightSpaceMatrix;

void main()
{
    vec3 off = vec3(0.01, 0.01, 0.0);
    vec3 offsetHeightR = vec3(position.x + 1.0, position.y + texture(heightmap, uvs + off.xz).y * heightCoef, position.z);
    vec3 offsetHeightD = vec3(position.x, position.y + texture(heightmap, uvs - off.zy).y * heightCoef, position.z - 1.0);

    uvs = texcoords;
    fragPos = vec3(model * vec4(position, 1.0));
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    float height = texture(heightmap, uvs).y * heightCoef;
    vec3 adjustedPos = vec3(position.x, position.y + height, position.z);
    vec3 adjustedNormal = normalize(cross(offsetHeightR - adjustedPos, offsetHeightD - adjustedPos));
    fragNormal = normalize(mat3(normalMat) * adjustedNormal);
    gl_Position = proj * view * model * vec4(position.x, position.y + height, position.z, 1.0);
}