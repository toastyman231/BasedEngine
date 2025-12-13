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
    vec4 full_albedo = GetMaterialAlbedo(uvs);
    vec3 albedo = full_albedo.rgb;
    float alpha = full_albedo.a;

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(vec3(eyePos) - fragPos);
    if (material.normal.useSampler == 1) {
        N = vec3(GetMaterialNormal(uvs));
        N = normalize(TBN * N);
    }

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 X = normalize(TBN[0]);
    vec3 Y = normalize(TBN[1]);

    vec3 Lo = vec3(0.0);

    float shadow = CalculateShadows(fragPosLightSpace);

    /**for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        PointLight light = pointLights[i];
        if (light.color == vec3(0.0)) continue;
        vec3 lightPos = light.position;
        vec3 lightColor = vec3(light.color);

        vec3 L = normalize(lightPos - fragPos);

        BRDFResults reflection = DisneyBRDF(albedo, L, V, N, X, Y);

        Lo += (lightColor * (reflection.diffuse + reflection.specular + reflection.clearcoat)) * DotClamped(N, L) * (1.0 - shadow) * light.intensity;
    }*/

    vec3 Do = vec3(0.0);
    float intensityMult = 1.0;
    
    if (directionalLight.color != vec3(0.0)) {
        vec3 L = normalize(-directionalLight.direction);
        vec3 lightColor = directionalLight.color; // TODO: Consider multiplying intensity here
        vec3 H = normalize(L + V);

        float NdotL = DotClamped(N, L);

        BRDFResults res;
        res = DisneyBRDF(albedo, L, V, N, X, Y);
        
        if (renderMode == RenderMode_DiffuseDebug) {
            Do += res.diffuse;
        } else if (renderMode == RenderMode_SpecularDebug) {
            Do += res.specular;
        } else if (renderMode == RenderMode_ClearCoatDebug) {
            Do += res.clearcoat;
        } else {
            Do += lightColor * (res.diffuse + res.specular + res.clearcoat) * NdotL /*** (1.0 - shadow)*/;
        }
        
        //outColor = vec4(res.specular, 1.0);
        intensityMult = directionalLight.intensity;
    }
    
    vec3 color;
    if (renderMode == RenderMode_DiffuseDebug || renderMode == RenderMode_SpecularDebug ||
        renderMode == RenderMode_ClearCoatDebug) {
        color = Do;
    } else {
        float ao = GetMaterialAmbientOcclusion(uvs);
        vec3 ambient = vec3(ambientStrength) * albedo;
        if (ao > 0.0) ambient *= ao;
        color = ambient + Do + Lo;
        color = color / (color + vec3(1.0));
        color *= intensityMult;
        color = LinearToSRGB(color);
    }
    
    if (renderMode == RenderMode_Lit) {
        if (material.blendMode == BlendMode_Masked && alpha < 0.1) discard; 
        outColor = vec4(color + GetMaterialEmission(uvs).xyz, material.blendMode == BlendMode_Opaque ? 1.0 : alpha);
    } else if (renderMode == RenderMode_Unlit) {
        if (material.blendMode == BlendMode_Masked && alpha < 0.1) discard; 
        outColor = vec4(albedo + GetMaterialEmission(uvs).xyz, material.blendMode == BlendMode_Opaque ? 1.0 : alpha);
    } else if (renderMode == RenderMode_NormalDebug) {
        outColor = vec4((N + 1.0) / 2.0, 1.0);
    } else if (renderMode == RenderMode_MetallicDebug) {
        outColor = vec4(vec3(max(Metallic, GetMaterialMetallic(uvs))), 1.0);
    } else if (renderMode == RenderMode_RoughnessDebug) {
        outColor = vec4(vec3(max(Roughness, GetMaterialRoughness(uvs))), 1.0);
    } else if (renderMode == RenderMode_AmbientOcclusionDebug) {
        outColor = vec4(vec3(GetMaterialAmbientOcclusion(uvs)), 1.0);
    } else if (renderMode == RenderMode_EmissionDebug) {
        outColor = vec4(vec3(GetMaterialEmission(uvs)), 1.0);
    } else if (renderMode == RenderMode_DiffuseDebug) {
        outColor = vec4(color, 1.0);
    } else if (renderMode == RenderMode_SpecularDebug) {
        outColor = vec4(color, 1.0);
    } else if (renderMode == RenderMode_ClearCoatDebug) {
        outColor = vec4(color, 1.0);
    } else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
    }
}