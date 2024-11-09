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

#include "material.glsl"
#include "globals.glsl"
uniform mat4 model = mat4(1.0);
uniform mat4 normalMat = mat4(1.0);
uniform mat4 overrideProj = mat4(1.0);
uniform mat4 overrideView = mat4(1.0);
uniform int useOverrideMatrices = 0;
uniform mat4 lightSpaceMatrix;

// vertex: the vertex to be snapped (needs to be in projection-space)
// resolution: the lower resolution, e.g. if my screen resolution is 1280x720, I might choose 640x320
vec4 snap(vec4 vertex, vec2 resolution)
{
    vec4 snappedPos = vertex;
    snappedPos.xyz = vertex.xyz / vertex.w; // convert to normalised device coordinates (NDC)
    snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
    snappedPos.xyz *= vertex.w; // convert back to projection-space
    return snappedPos;
}

void main()
{
    uvs = texcoords;
    fragNormal = normalize(mat3(normalMat) * normal);
    fragPos = vec3(model * vec4(position, 1.0));
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    //vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

    //vec4 pos = snap(proj * view * model * vec4(position, 1.0), vec2(160.0, 120.0));

    if (useOverrideMatrices == 0) {
        gl_Position = proj * view * model * vec4(position, 1.0);
    } else {
        gl_Position = overrideProj * overrideView * model * vec4(position, 1.0);
    }
}