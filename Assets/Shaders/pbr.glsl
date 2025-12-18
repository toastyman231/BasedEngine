#include "color.glsl"

struct MaterialProperty {
    vec4 color;
    vec4 tint;
    int useSampler;
    sampler2D tex;
};

const int RenderMode_Lit                   = 0;
const int RenderMode_Unlit                 = 1;
const int RenderMode_NormalDebug           = 2;
const int RenderMode_MetallicDebug         = 3;
const int RenderMode_RoughnessDebug        = 4;
const int RenderMode_AmbientOcclusionDebug = 5;
const int RenderMode_EmissionDebug         = 6;
const int RenderMode_DiffuseDebug          = 7;
const int RenderMode_SpecularDebug         = 8;
const int RenderMode_ClearCoatDebug        = 9;

const int BlendMode_Opaque                 = 0;
const int BlendMode_Masked                 = 1;
const int BlendMode_Translucent            = 2;

struct Material {
    MaterialProperty albedo;
    MaterialProperty normal;
    MaterialProperty metallic;
    MaterialProperty roughness;
    MaterialProperty ambientOcclusion;
    MaterialProperty emission;
    int deriveNormalZ;
    int blendMode;
};

uniform Material material;

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    float intensity;

    vec3 color;
};
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};
#define NR_POINT_LIGHTS 8
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirectionalLight directionalLight;
uniform int castShadows = 1;
uniform int receiveShadows = 1;

const float PI = 3.14159265359;

uniform float ambientStrength = 0.1;

uniform sampler2D ShadowMap;

// MATERIAL FUNCS

vec4 GetMaterialColor(MaterialProperty mat, vec2 uv) {
    int shouldUseSampler = mat.useSampler;
    vec4 color = (1-shouldUseSampler) * mat.color;
    vec4 tex =   shouldUseSampler     * texture(mat.tex, uv);
    vec4 tint = mat.tint;
    // if useSampler is true, color is 0, if it's false, tex is 0
    if (tint.r + tint.g + tint.b == 0) {
        tint = vec4(1.0);
    }
    return (color + tex) * tint;
}

vec4 GetMaterialAlbedo(vec2 uv) {
    vec4 color = GetMaterialColor(material.albedo, uv);
    return vec4(SRGBToLinear(color.rgb), color.a);
}

vec4 GetMaterialNormal(vec2 uv) {
    vec4 normal = GetMaterialColor(material.normal, uv);
    normal = normal * 2.0 - 1.0;
    
    if (material.deriveNormalZ > 0) {
        vec2 normalXY = normal.rg;
        float normalZ = sqrt(clamp(1.0 - dot(normalXY, normalXY), 0.0, 1.0));
        return vec4(normalXY.xy, normalZ, 1.0);
    } 
    else {
        return normal;
    }
}

float GetMaterialMetallic(vec2 uv) {
    vec4 metallic = GetMaterialColor(material.metallic, uv);
    if (metallic.r != 0.0) return metallic.r;
    return metallic.b;
}

float GetMaterialRoughness(vec2 uv) {
    vec4 roughness = GetMaterialColor(material.roughness, uv);
    if (roughness.r != 0.0) return roughness.r;
    return roughness.g;
}

float GetMaterialAmbientOcclusion(vec2 uv) {
    vec4 ao = GetMaterialColor(material.ambientOcclusion, uv);
    return ao.r;
}

vec4 GetMaterialEmission(vec2 uv) {
    return GetMaterialColor(material.emission, uv);
}

float CalculateShadows(vec4 fragPosLightSpace) {
    if (receiveShadows == 0) return 0.0;

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = 0.005;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}

// DISNEY VERSIONS

uniform float Roughness = 1.0;
uniform float Metallic = 0;
uniform float Subsurface = 0.0;
uniform float Specular = 0.5;
uniform float SpecularTint = 0.0;
uniform float Anisotropic = 0.0;
uniform float Sheen = 0.0;
uniform float SheenTint = 0.5;
uniform float ClearCoat = 0.0;
uniform float ClearCoatGloss = 1.0;

uniform float NormalStrength = 1.0;

float sqr(float x) {
    return x * x;
}

float CalculateLuminance(vec3 baseColor) {
    return dot(vec3(0.299, 0.587, 0.114), baseColor);
}

vec3 CalculateTint(vec3 baseColor) {
    float luminance = CalculateLuminance(baseColor);
    return (luminance > 0.0) ? baseColor * (1.0 / luminance) : vec3(1);
}

float SchlickFresnel(float x) {
    float m = clamp(1.0 - x, 0.0, 1.0);
    float m2 = m * m;
    return m * m2 * m2;
}

float Schlick(float r0, float rad) {
    return mix(1.0, SchlickFresnel(rad), r0);
}

vec3 Schlick(vec3 r0, float rad) {
    float exponential = pow(1.0 - rad, 5.0);
    return r0 + (vec3(1.0) - r0) * exponential;
}

float SchlickR0FromRelativeIOR(float eta)
{
    // https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
    return sqr(eta - 1.0f) / sqr(eta + 1.0f);
}

float GTR1(float NdotH, float a) {
    if (a >= 1) return 1.0 / PI;

    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    float result = (a2 - 1.0) / (PI * log(a2) * t);
    return clamp(result, 0.0, 1000.0);
}

float AnisotropicGTR2(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1.0 / (PI * ax * ay * sqr(sqr(HdotX / ax) + sqr(HdotY / ay) + sqr(NdotH)));
}

float SmithGGX(float alphaSquared, float ndotl, float ndotv) {
    float a = ndotv * sqrt(alphaSquared + ndotl * (ndotl - alphaSquared * ndotl));
    float b = ndotl * sqrt(alphaSquared + ndotv * (ndotv - alphaSquared * ndotv));

    return 0.5f / (a + b);
}

float SeparableSmithGGXG1(float NdotV, float a)
{
    float a2 = a * a;
    float absDotNV = abs(NdotV);

    return 2.0f / (1.0f + sqrt(a2 + (1 - a2) * absDotNV * absDotNV));
}

float AnisotropicSmithGGX(float NdotS, float SdotX, float SdotY, float ax, float ay) {
    return 1.0 / (NdotS + sqrt(sqr(SdotX * ax) + sqr(SdotY * ay) + sqr(NdotS)));
}

float DotClamped(vec3 x, vec3 y) {
    return clamp(dot(x, y), 0.0, 1.0);
}

struct BRDFResults {
    vec3 diffuse;
    vec3 specular;
    vec3 clearcoat;
};

BRDFResults DisneyBRDF(vec3 baseColor, vec3 L, vec3 V, vec3 N, vec3 X, vec3 Y) {
    BRDFResults brdfResult;
    brdfResult.diffuse = vec3(0.0);
    brdfResult.specular = vec3(0.0);
    brdfResult.clearcoat = vec3(0.0);

    float rough = Roughness * GetMaterialRoughness(uvs);
    float metal = Metallic * GetMaterialMetallic(uvs);

    vec3 H = normalize(L + V);

    float NdotL = DotClamped(N, L);
    float NdotV = DotClamped(N, V);
    float NdotH = DotClamped(N, H);
    float LdotH = DotClamped(L, H);
    
    // Sheen lobe: Gives the surface a sheen at grazing angles, optionally tinted based on the surface color
    // Sheen - Sheen lobe strength
    // SheenTint - Sheen color strength
    vec3 Cdlin = baseColor;// We expect this in linear space already // * material.albedo.tint; TODO: ADD THIS BACK IN LATER
    float Cdlum = CalculateLuminance(Cdlin);
    vec3 Ctint = Cdlum > 0.0 ? Cdlin / Cdlum : vec3(1.0);
    vec3 Csheen = mix(vec3(1.0), Ctint, SheenTint);
    float FH = SchlickFresnel(LdotH);
    vec3 Fsheen = FH * Sheen * Csheen;
    
    // Diffuse lobe: Light reflected off the surface in many directions depending on Roughness
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    
    // Fresnel term: Greater reflectance at grazing angles
    float Fss90 = LdotH * LdotH * rough;
    float Fd90 = 0.5 + 2.0 * Fss90;

    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // Subsurface Scattering approximation (Hanrahan-Krueger)
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5);
    
    // Specular lobe: Light reflected back towards the viewer
    // Light is more likely to be reflected when the microfacet normal aligns w/
    // the vector of ideal reflectance (H)
    // Anisotropic - Microfacet distribution varies based on axis, making specular highlights more elliptical
    float alpha = rough;
    float alphaSquared = alpha * alpha;

    // Anisotropic Microfacet Normal Distribution (Normalized Anisotropic GTR gamma == 2)
    float aspectRatio = sqrt(1.0 - Anisotropic * 0.9);
    float alphaX = max(0.001, alphaSquared / aspectRatio);
    float alphaY = max(0.001, alphaSquared * aspectRatio);
    float Ds = AnisotropicGTR2(NdotH, dot(H, X), dot(H, Y), alphaX, alphaY);

    // Geometric Attenuation (shadowing and masking due to microfacets)
    float GalphaSquared = sqr(0.5 + rough * 0.5);
    float GalphaX = max(0.001, GalphaSquared / aspectRatio);
    float GalphaY = max(0.001, GalphaSquared * aspectRatio);
    float G = AnisotropicSmithGGX(NdotL, dot(L, X), dot(L, Y), GalphaX, GalphaY);
    G *= AnisotropicSmithGGX(NdotV, dot(V, X), dot(V, Y), GalphaX, GalphaY);

    vec3 Cspec0 = mix(Specular * 0.08 * mix(vec3(1.0), Ctint, SpecularTint), Cdlin, metal);
    vec3 F = mix(Cspec0, vec3(1.0), FH);

    // Clearcoat lobe: Mixes in a clear coating on top of the base color
    // ClearCoat - lobe intensity
    // ClearCoatGloss - lobe shape
    float Dr = GTR1(NdotH, mix(0.1, 0.02, ClearCoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = SmithGGX(NdotL, NdotV, 0.25);

    brdfResult.diffuse = (1.0 / PI) * (mix(Fd, ss, Subsurface) * Cdlin + Fsheen) * (1 - metal);
    brdfResult.specular = Ds * F * G;/** / (4.0 * NdotL * NdotV)*/;
    brdfResult.clearcoat = clamp(vec3(0.25 * ClearCoat * Gr * Fr * Dr), vec3(0.0), vec3(10.0));

    return brdfResult;
}