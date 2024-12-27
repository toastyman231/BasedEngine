#version 410 core

out vec4 outColor;

in vec2 uvs;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;
in mat4 objModel;

#include "material.glsl"
#include "globals.glsl"
#include "lighting.glsl"

const float PI = 3.141596;

uniform float _FragmentSeed, _FragmentSeedIter, _FragmentFrequency, _FragmentFrequencyMult, _FragmentAmplitude, _FragmentAmplitudeMult, _FragmentInitialSpeed, _FragmentSpeedRamp, _FragmentDrag, _FragmentHeight, _FragmentMaxPeak, _FragmentPeakOffset;
uniform float _NormalStrength, _FresnelNormalStrength, _SpecularNormalStrength;

uniform vec3 waterColor = vec3(3.0/255.0, 140.0/255.0, 252.0/255.0);

uniform vec3 _Ambient, _DiffuseReflectance, _SpecularReflectance, _FresnelColor, _TipColor;
uniform float _Shininess, _FresnelBias, _FresnelStrength, _FresnelShininess, _TipAttenuation;

uniform int _FragmentWaveCount;

vec3 fragmentFBM(vec3 v) {
    float f = _FragmentFrequency;
    float a = _FragmentAmplitude;
    float speed = _FragmentInitialSpeed;
    float seed = _FragmentSeed;
    vec3 p = v;

    float h = 0.0;
    vec2 n = vec2(0.0);
    
    float amplitudeSum = 0.0;

    for (int wi = 0; wi < _FragmentWaveCount; ++wi) {
        vec2 d = normalize(vec2(cos(seed), sin(seed)));

        float x = dot(d, p.xz) * f + time * speed;
        float wave = a * exp(_FragmentMaxPeak * sin(x) - _FragmentPeakOffset);
        vec2 dw = f * d * (_FragmentMaxPeak * wave * cos(x));
        
        h += wave;
        p.xz += -dw * a * _FragmentDrag;
        
        n += dw;
        
        amplitudeSum += a;
        f *= _FragmentFrequencyMult;
        a *= _FragmentAmplitudeMult;
        speed *= _FragmentSpeedRamp;
        seed += _FragmentSeedIter;
    }
    
    vec3 result = vec3(h, n.x, n.y) / amplitudeSum;
    result.x *= _FragmentHeight;

    return result;
}

float DotClamped(vec3 a, vec3 b) {
    float result = dot(a, b);
    if (result < 0.0) return 0.0;
    else return result;
}

void main()
{
    vec3 lightDir = -normalize(directionalLight.direction);
    vec3 viewDir = normalize(vec3(eyePos) - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec3 normal = fragNormal;
    float height = 0.0;

    vec3 fbm = fragmentFBM(fragPos);
    height = fbm.x;
    normal.xy = fbm.yz;

    normal = normalize(vec3(objModel * normalize(vec4(-normal.x, 1.0, -normal.y, 0.0))));
    //normal.xz *= _NormalStrength;
    //normal = normalize(normal);

    float ndotl = DotClamped(lightDir, normal);

    vec3 diffuseReflectance = _DiffuseReflectance / PI;
    vec3 diffuse = pointLights[1].color * ndotl * diffuseReflectance;

    // Schlick Fresnel
    vec3 fresnelNormal = normal;
    fresnelNormal.xz *= _FresnelNormalStrength;
    fresnelNormal = normalize(fresnelNormal);
    float base = 1 - dot(viewDir, fresnelNormal);
    float exponential = pow(base, _FresnelShininess);
    float R = exponential + _FresnelBias * (1.0 - exponential);
    R *= _FresnelStrength;
    
    vec3 fresnel = _FresnelColor * R;

    vec3 specularReflectance = _SpecularReflectance;
    vec3 specNormal = normal;
    specNormal.xz *= _SpecularNormalStrength;
    specNormal = normalize(specNormal);
    float spec = pow(DotClamped(specNormal, halfwayDir), _Shininess) * ndotl;
    vec3 specular = pointLights[1].color * specularReflectance * spec;

    // Schlick Fresnel but again for specular
    base = 1 - DotClamped(viewDir, halfwayDir);
    exponential = pow(base, 5.0);
    R = exponential + _FresnelBias * (1.0 - exponential);

    specular *= R;

    vec3 tipColor = _TipColor * pow(height, _TipAttenuation);

    vec3 result = _Ambient + diffuse + specular + fresnel + tipColor;
    //result *= CalculateLighting(material, uvs, normal, fragPos, viewDir, fragPosLightSpace);

    outColor = vec4(result, 1.0);
}