#include "material.glsl"

uniform float ambientStrength = 0.1;

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
uniform int useLight = 1;

vec3 CalculateDirectionalLighting(DirectionalLight light, vec3 normal, vec3 viewDir, float shininess) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient  = light.color * ambientStrength;
    vec3 diffuse  = light.color * diff;
    vec3 specular = light.color * spec;
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLighting(PointLight light, Material material, vec3 normal, vec3 fragPos, vec3 viewDir, float shininess) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = 1.0/distance(fragPos, light.position);//max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    // attenuation
    float dist    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + 
  			     light.quadratic * (dist * dist));    
    // combine results
    vec3 ambient = light.color * ambientStrength;
    vec3 diffuse  = (light.color * light.intensity) * diff;
    vec3 specular = light.color * spec;
    ambient *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalculateLighting(Material material, vec2 uvs, vec3 normal, vec3 fragPos, vec3 viewDir) {
    float shininess = GetShininessMaterial(uvs);

    vec3 result = vec3(0);

    if (directionalLight.direction != vec3(0)) {
        result += CalculateDirectionalLighting(directionalLight, normal, viewDir, shininess);
    }
    for (int i = 0; i < 8; i++) {
        if (pointLights[i].color == vec3(0)) continue;
        result += CalculatePointLighting(pointLights[i], material, normal, fragPos, viewDir, shininess);
    }

    return useLight == 1 ? result * GetDiffuseMaterial(uvs).rgb : GetDiffuseMaterial(uvs).rgb;
}