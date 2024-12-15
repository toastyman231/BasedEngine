struct MaterialProperty {
    vec4 color;
    vec4 tint;
    int useSampler;
    sampler2D tex;
};
struct Material {
    MaterialProperty albedo;
    MaterialProperty normal;
    MaterialProperty metallic;
    MaterialProperty roughness;
    MaterialProperty ambientOcclusion;
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
};
#define NR_POINT_LIGHTS 8
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirectionalLight directionalLight;
uniform int castShadows = 1;
uniform int receiveShadows = 1;

const float PI = 3.14159265359;

uniform float ambientStrength = 0.1;

uniform sampler2D shadowMap;

float CalculateShadows(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = 0.005;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if (projCoords.z > 1.0 || receiveShadows == 0)
        shadow = 0.0;

    return shadow;
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// DISNEY VERSIONS

uniform float Metallic = 0.0;
uniform float Subsurface = 0.0;
uniform float Specular = 0.5;
uniform float Roughness = 0.5;
uniform float SpecularTint = 0.0;
uniform float Anisotropic = 0.0;
uniform float Sheen = 0.0;
uniform float SheenTint = 0.5;
uniform float ClearCoat = 0.0;
uniform float ClearCoatGloss = 1.0;

float sqr(float x) {
    return x * x;
}

float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

float SchlickFresnel(float x) {
    x = clamp(1.0 - x, 0.0, 1.0);
    float x2 = x * x;
    return x2 * x2 * x;
}

float GTR1(float NdotH, float a) {
    float a2 = a * a;
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}

float AnisotropicGTR2(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1.0 / (PI * ax * ay * sqr(sqr(HdotX / ax) + sqr(HdotY / ay) + sqr(NdotH)));
}

float SmithGGX(float alphaSquared, float NdotL, float NdotV) {
    float a = NdotV * sqrt(alphaSquared + NdotL * (NdotL - alphaSquared * NdotL));
    float b = NdotL * sqrt(alphaSquared + NdotV * (NdotV - alphaSquared * NdotV));

    return 0.5 / (a + b);
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

    vec3 H = normalize(L + V);

    float NdotL = DotClamped(N, L);
    float NdotV = DotClamped(N, V);
    float NdotH = DotClamped(N, H);
    float LdotH = DotClamped(L, H);

    vec3 surfaceColor = baseColor;// * material.albedo.tint; TODO: ADD THIS BACK IN LATER
    float Cdlum = luminance(surfaceColor);

    vec3 Ctint = Cdlum > 0.0 ? surfaceColor / Cdlum : vec3(1.0);
    vec3 Cspec0 = mix(Specular * 0.08 * mix(vec3(1.0), Ctint, SpecularTint), surfaceColor, Metallic);
    vec3 Csheen = mix(vec3(1.0), Ctint, SheenTint);

    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);

    float Fss90 = LdotH * LdotH * Roughness;
    float Fd90 = 0.5 + 2.0 * Fss90;

    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5);

    float alpha = Roughness;
    float alphaSquared = alpha * alpha;

    float aspectRatio = sqrt(1.0 - Anisotropic * 0.9);
    float alphaX = max(0.001, alphaSquared / aspectRatio);
    float alphaY = max(0.001, alphaSquared * aspectRatio);
    float Ds = AnisotropicGTR2(NdotH, dot(H, X), dot(H, Y), alphaX, alphaY);

    float GalphaSquared = sqr(0.5 + Roughness * 0.5);
    float GalphaX = max(0.001, GalphaSquared / aspectRatio);
    float GalphaY = max(0.001, GalphaSquared * aspectRatio);
    float G = AnisotropicSmithGGX(NdotL, dot(L, X), dot(L, Y), GalphaX, GalphaY);
    G *= AnisotropicSmithGGX(NdotV, dot(V, X), dot(V, Y), GalphaX, GalphaY);

    float FH = SchlickFresnel(LdotH);
    vec3 F = mix(Cspec0, vec3(1.0), FH);

    vec3 Fsheen = FH * Sheen * Csheen;

    float Dr = GTR1(NdotH, mix(0.1, 0.001, ClearCoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = SmithGGX(0.25, NdotL, NdotV);

    brdfResult.diffuse = (1.0 / PI) * (mix(Fd, ss, Subsurface) * surfaceColor + Fsheen) * (1 - Metallic);
    brdfResult.specular = Ds * F * G;
    brdfResult.clearcoat = vec3(0.25 * ClearCoat * Gr * Fr * Dr);

    return brdfResult;
}

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
    vec3 color = GetMaterialColor(material.albedo, uv).rgb;
    return vec4(pow(color.r, 2.2), pow(color.g, 2.2), pow(color.b, 2.2), 1.0);
}

vec4 GetMaterialNormal(vec2 uv) {
    return GetMaterialColor(material.normal, uv);
}

float GetMaterialMetallic(vec2 uv) {
    return GetMaterialColor(material.metallic, uv).r;
}

float GetMaterialRoughness(vec2 uv) {
    return GetMaterialColor(material.roughness, uv).a;
}

float GetMaterialAmbientOcclusion(vec2 uv) {
    return GetMaterialColor(material.ambientOcclusion, uv).r;
}