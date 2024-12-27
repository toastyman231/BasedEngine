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
out mat3 TBN;
out mat4 objModel;

#include "material.glsl"
#include "globals.glsl"
uniform mat4 model = mat4(1.0);
uniform mat4 normalMat = mat4(1.0);
uniform mat4 overrideProj = mat4(1.0);
uniform mat4 overrideView = mat4(1.0);
uniform int useOverrideMatrices = 0;
uniform mat4 lightSpaceMatrix;

uniform float _VertexSeed, _VertexSeedIter, _VertexFrequency, _VertexFrequencyMult, _VertexAmplitude, _VertexAmplitudeMult, _VertexInitialSpeed, _VertexSpeedRamp, _VertexDrag, _VertexHeight, _VertexMaxPeak, _VertexPeakOffset;
uniform float _NormalStrength, _FresnelNormalStrength, _SpecularNormalStrength;

uniform int _VertexWaveCount;

vec3 vertexFBM(vec3 v) {
    float f = _VertexFrequency;
    float a = _VertexAmplitude;
    float speed = _VertexInitialSpeed;
    float seed = _VertexSeed;
    vec3 p = v;
    float amplitudeSum = 0.0;

    float h = 0.0;
    vec2 n = vec2(0.0);
    for (int wi = 0; wi < _VertexWaveCount; ++wi) {
        vec2 d = normalize(vec2(cos(seed), sin(seed)));

        float x = dot(d, p.xz) * f + time * speed;
        float wave = a * exp(_VertexMaxPeak * sin(x) - _VertexPeakOffset);
        float dx = _VertexMaxPeak * wave * cos(x);
        
        h += wave;
        
        p.xz += d * -dx * a * _VertexDrag;

        amplitudeSum += a;
        f *= _VertexFrequencyMult;
        a *= _VertexAmplitudeMult;
        speed *= _VertexSpeedRamp;
        seed += _VertexSeedIter;
    }

    vec3 result = vec3(h, n.x, n.y) / amplitudeSum;
    result.x *= _VertexHeight;

    return result;
}

void main()
{
    uvs = texcoords;
    fragNormal = normalize(mat3(normalMat) * normal);
    fragPos = vec3(model * vec4(position, 1.0));
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

    vec3 h = vec3(0.0);
    vec3 n = vec3(0.0);

    vec3 fbm = vertexFBM(fragPos);

    h.y = fbm.x;
    n.xy = fbm.yz;

    vec4 newPos = vec4(position, 1.0) + vec4(h, 0.0);
    fragPos = vec3(model * newPos);
    objModel = model;

    if (useOverrideMatrices == 0) {
        gl_Position = proj * view * model * newPos;
    } else {
        gl_Position = overrideProj * overrideView * model * vec4(position, 1.0);
    }
}