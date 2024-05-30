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
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        PointLight light = pointLights[i];
        if (light.color == vec3(0.0)) continue;
        vec3 lightPos = light.position;
        vec3 lightColor = vec3(light.color);

        vec3 L = normalize(lightPos);

        BRDFResults reflection = DisneyBRDF(albedo, L, V, N, X, Y);

        Lo += (lightColor * (reflection.diffuse + reflection.specular + reflection.clearcoat)) * DotClamped(N, L);

        /*vec3 L = normalize(lightPos - fragPos);
        vec3 H = normalize(V + L);

        float dist = length(lightPos - fragPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance = lightColor * attenuation;
        vec3 F = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
        
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;*/
    }

    vec3 ambient = vec3(ambientStrength) * albedo * GetMaterialAmbientOcclusion(uvs);
    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}