#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 7) in mat4 instanceModel;

out vec2 uvs;
out vec3 fragNormal;
out vec3 fragPos;
out vec4 fragPosLightSpace;
out float heightPercent;

uniform sampler2D height;
#include "globals.glsl"
#include "noise.glsl"
uniform float randomLean;
uniform float heightCoef;
uniform mat4 lightSpaceMatrix;

mat4 rotateX(float angle) {
    mat4 rotMatrix;
    rotMatrix[0] = vec4(1, 0, 0, 0);
    rotMatrix[1] = vec4(0, cos(angle), sin(angle), 0);
    rotMatrix[2] = vec4(0, -sin(angle), cos(angle), 0);
    rotMatrix[3] = vec4(0, 0, 0, 1);
    return rotMatrix;
}

mat4 rotateY(float angle) {
    mat4 rotMatrix;
    rotMatrix[0] = vec4(cos(angle), 0, -sin(angle), 0);
    rotMatrix[1] = vec4(0, 1, 0, 0);
    rotMatrix[2] = vec4(sin(angle), 0, cos(angle), 0);
    rotMatrix[3] = vec4(0, 0, 0, 1);
    return rotMatrix;
}

float map(float value, float min1, float max1, float min2, float max2) {
    // Convert the current value to a percentage
    // 0% - min1, 100% - max1
    float perc = (value - min1) / (max1 - min1);

    // Do the same operation backwards with min2 and max2
    return perc * (max2 - min2) + min2;
}

float EaseOutSine(float x)
{
    return sin((x * 3.14159) / 2.0);
}

float saturate(float value) {
    return clamp(value, 0.0, 1.0);
}

void main()
{
    vec3 rotatedNormal1 = vec3(rotateY(3.14159 * 0.3) * vec4(normal, 1.0));
    vec3 rotatedNormal2 = vec3(rotateY(3.14159 * -0.3) * vec4(normal, 1.0));

    float mixFactor = EaseOutSine(position.x + 1 - 0.5);

    vec3 adjustedNormal = mix(rotatedNormal1, rotatedNormal2, mixFactor);
    vec4 worldPos = instanceModel * vec4(position, 1.0);

    heightPercent = position.y / 0.601094;

    float curveAmount = randomLean * heightPercent;
    float noiseSample = cnoise(vec2(time * 0.35) + worldPos.xz);
    curveAmount += noiseSample * 0.1;

    float windDir = cnoise(worldPos.xz * 0.05 + 0.05 * time);
    windDir = map(windDir, -1.0, 1.0, 0.0, 3.14159 * 2.0);

    float windNoiseSample = cnoise(worldPos.xz * 0.25 + time);

    float windLeanAngle = map(windNoiseSample, -1.0, 1.0, 0.25, 1.0);

    mat4 grassMat = rotateX(curveAmount);
    mat4 windMat = rotateX(windLeanAngle);
    vec4 pos = grassMat * windMat * vec4(position, 1.0);

    uvs = texcoords;
    mat3 normalMat = mat3(transpose(inverse(instanceModel)));
    fragNormal = normalize(normalMat * adjustedNormal);
    fragPos = vec3(instanceModel * vec4(position, 1.0));
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    float heightOffset = texture(height, vec2(map(fragPos.x, -100.0, 100.0, 0.0, 1.0), map(fragPos.z, -100.0, 100.0, 1.0, 0.0))).y * heightCoef;
    pos.y += heightOffset;
    gl_Position = proj * view * instanceModel * pos;
}