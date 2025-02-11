#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;
in mat3 TBN;

#include "globals.glsl"
#include "pbr.glsl"

void main() {
    float roughness = GetMaterialRoughness(uvs);
    float metallic = GetMaterialMetallic(uvs);
    vec3 albedo = vec3(GetMaterialAlbedo(uvs));

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(vec3(eyePos) - fragPos);
    if (material.normal.useSampler == 1) {
        N = vec3(GetMaterialNormal(uvs));
        N = N * 2.0 - 1.0;
        N = normalize(TBN * N);
    }

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 X = normalize(TBN[0]);
    vec3 Y = normalize(cross(N, TBN[0]));

    vec3 Lo = vec3(0.0);

    float shadow = CalculateShadows(fragPosLightSpace);

    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        PointLight light = pointLights[i];
        if (light.color == vec3(0.0)) continue;
        vec3 lightPos = light.position;
        vec3 lightColor = vec3(light.color);

        vec3 L = normalize(lightPos);

        BRDFResults reflection = DisneyBRDF(albedo, L, V, N, X, Y);

        Lo += (lightColor * (reflection.diffuse + reflection.specular + reflection.clearcoat)) * DotClamped(N, L) * (1.0 - shadow);
    }

    vec3 Do = vec3(0.0);
    float intensityMult = 1.0;

    if (directionalLight.direction != vec3(0)) {
        if (directionalLight.color != vec3(0.0)) {
            vec3 L = normalize(-directionalLight.direction);
            vec3 lightColor = directionalLight.color;
            vec3 H = normalize(L + V);

            float NdotL = DotClamped(N, L);

            BRDFResults res = DisneyBRDF(albedo, L, V, N, X, Y);

            Do += (lightColor * (res.diffuse + res.specular + res.clearcoat)) * (NdotL + ambientStrength) * (1.0 - shadow);
            intensityMult = directionalLight.intensity;
        }
    }

    float ao = GetMaterialAmbientOcclusion(uvs);
    vec3 ambient = vec3(ambientStrength) * albedo;
    if (ao > 0.0) ambient *= ao;
    vec3 color = ambient + Do + Lo;
    color *= intensityMult;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    if (renderMode == 0) {
        outColor = vec4(color, 1.0);
    } else if (renderMode == 1) {
        outColor = vec4(albedo, 1.0);
    } else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
}