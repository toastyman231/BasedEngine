#pragma once
#include <memory>

#include "based/graphics/material.h"

inline std::shared_ptr<based::graphics::Material> waterMaterial = nullptr;

inline int normalStrength = 1;
inline int vertexWaveCount = 8;
inline int fragmentWaveCount = 40;

inline float vertexSeed = 0;
inline float vertexSeedIter = 1253.2131f;
inline float vertexFrequency = 1.0f;
inline float vertexFrequencyMult = 1.18f;
inline float vertexAmplitude = 1.0f;
inline float vertexAmplitudeMult = 0.82f;
inline float vertexInitialSpeed = 2.0f;
inline float vertexSpeedRamp = 1.07f;
inline float vertexDrag = 1.0f;
inline float vertexHeight = 1.0f;
inline float vertexMaxPeak = 1.0f;
inline float vertexPeakOffset = 1.0f;
inline float fragmentSeed = 0;
inline float fragmentSeedIter = 1253.2131f;
inline float fragmentFrequency = 1.0f;
inline float fragmentFrequencyMult = 1.18f;
inline float fragmentAmplitude = 1.0f;
inline float fragmentAmplitudeMult = 0.82f;
inline float fragmentInitialSpeed = 2.0f;
inline float fragmentSpeedRamp = 1.07f;
inline float fragmentDrag = 1.0f;
inline float fragmentHeight = 1.0f;
inline float fragmentMaxPeak = 1.0f;
inline float fragmentPeakOffset = 1.0f;

inline glm::vec3 ambient = glm::vec3(0.f, 0.213f, 0.981f);

inline glm::vec3 diffuseReflectance = glm::vec3(0, 0.19f, 1.f);

inline glm::vec3 specularReflectance = glm::vec3(0.f, 0.022f, 1.f);

inline float shininess = 100;
inline float specularNormalStrength = 1;

inline glm::vec3 fresnelColor = glm::vec3(0.f, 0.1f, 1.f);

inline float fresnelBias, fresnelStrength = 0.873f, fresnelShininess = 20.f;
inline float fresnelNormalStrength = 1;

inline glm::vec3 tipColor = glm::vec3(1.f);
inline float tipAttenuation = 3.88f;

inline void UpdateWaterShader()
{
	if (waterMaterial == nullptr) return;

    waterMaterial->SetUniformValue("_VertexWaveCount", vertexWaveCount);
    waterMaterial->SetUniformValue("_VertexSeed", vertexSeed);
    waterMaterial->SetUniformValue("_VertexSeedIter", vertexSeedIter);
    waterMaterial->SetUniformValue("_VertexFrequency", vertexFrequency);
    waterMaterial->SetUniformValue("_VertexFrequencyMult", vertexFrequencyMult);
    waterMaterial->SetUniformValue("_VertexAmplitude", vertexAmplitude);
    waterMaterial->SetUniformValue("_VertexAmplitudeMult", vertexAmplitudeMult);
    waterMaterial->SetUniformValue("_VertexInitialSpeed", vertexInitialSpeed);
    waterMaterial->SetUniformValue("_VertexSpeedRamp", vertexSpeedRamp);
    waterMaterial->SetUniformValue("_VertexDrag", vertexDrag);
    waterMaterial->SetUniformValue("_VertexHeight", vertexHeight);
    waterMaterial->SetUniformValue("_VertexMaxPeak", vertexMaxPeak);
    waterMaterial->SetUniformValue("_VertexPeakOffset", vertexPeakOffset);
    waterMaterial->SetUniformValue("_FragmentWaveCount", fragmentWaveCount);
    waterMaterial->SetUniformValue("_FragmentSeed", fragmentSeed);
    waterMaterial->SetUniformValue("_FragmentSeedIter", fragmentSeedIter);
    waterMaterial->SetUniformValue("_FragmentFrequency", fragmentFrequency);
    waterMaterial->SetUniformValue("_FragmentFrequencyMult", fragmentFrequencyMult);
    waterMaterial->SetUniformValue("_FragmentAmplitude", fragmentAmplitude);
    waterMaterial->SetUniformValue("_FragmentAmplitudeMult", fragmentAmplitudeMult);
    waterMaterial->SetUniformValue("_FragmentInitialSpeed", fragmentInitialSpeed);
    waterMaterial->SetUniformValue("_FragmentSpeedRamp", fragmentSpeedRamp);
    waterMaterial->SetUniformValue("_FragmentDrag", fragmentDrag);
    waterMaterial->SetUniformValue("_FragmentHeight", fragmentHeight);
    waterMaterial->SetUniformValue("_FragmentMaxPeak", fragmentMaxPeak);
    waterMaterial->SetUniformValue("_FragmentPeakOffset", fragmentPeakOffset);
    //waterMaterial->SetUniformValue("_NormalStrength", normalStrength);
    return;
}

inline void UpdateShaderVisuals()
{
    waterMaterial->SetUniformValue("_Ambient", ambient);
    waterMaterial->SetUniformValue("_DiffuseReflectance", diffuseReflectance);
    waterMaterial->SetUniformValue("_SpecularReflectance", specularReflectance);
    waterMaterial->SetUniformValue("_TipColor", tipColor);
    waterMaterial->SetUniformValue("_FresnelColor", fresnelColor);
    waterMaterial->SetUniformValue("_Shininess", shininess * 100);
    waterMaterial->SetUniformValue("_FresnelBias", fresnelBias);
    waterMaterial->SetUniformValue("_FresnelStrength", fresnelStrength);
    waterMaterial->SetUniformValue("_FresnelShininess", fresnelShininess);
    waterMaterial->SetUniformValue("_TipAttenuation", tipAttenuation);
    waterMaterial->SetUniformValue("_FresnelNormalStrength", fresnelNormalStrength);
    waterMaterial->SetUniformValue("_SpecularNormalStrength", specularNormalStrength);
}

inline void WaterSettings()
{
	if (ImGui::CollapsingHeader("Water Settings"))
	{
        ImGui::DragFloat("Vert Freq", &vertexFrequency, 0.1f);
        ImGui::DragFloat("Vert Amp", &vertexAmplitude, 0.1f);
        ImGui::DragFloat("Vert Speed", &vertexInitialSpeed, 0.1f);
        ImGui::DragFloat("Vert Seed", &vertexSeed, 0.1f);
        ImGui::DragInt("Vert Wave Count", &vertexWaveCount);
        ImGui::DragFloat("Vert Max Peak", &vertexMaxPeak, 0.1f);
        ImGui::DragFloat("Vert Peak Offset", &vertexPeakOffset, 0.1f);
        ImGui::DragFloat("Vert Drag", &vertexDrag, 0.1f);
        ImGui::DragFloat("Vert Freq Mult", &vertexFrequencyMult, 0.1f);
        ImGui::DragFloat("Vert Amp Mult", &vertexAmplitudeMult, 0.1f);
        ImGui::DragFloat("Vert Spd Ramp", &vertexSpeedRamp, 0.1f);
        ImGui::DragFloat("Vert Seed Iter", &vertexSeedIter, 0.1f);
        ImGui::DragFloat("Vert Height", &vertexHeight, 0.1f);

        ImGui::DragFloat("Frag Freq", &fragmentFrequency, 0.1f);
        ImGui::DragFloat("Frag Amp", &fragmentAmplitude, 0.1f);
        ImGui::DragFloat("Frag Speed", &fragmentInitialSpeed, 0.1f);
        ImGui::DragFloat("Frag Seed", &fragmentSeed, 0.1f);
        ImGui::DragInt("Frag Wave Count", &fragmentWaveCount);
        ImGui::DragFloat("Frag Max Peak", &fragmentMaxPeak, 0.1f);
        ImGui::DragFloat("Frag Peak Offset", &fragmentPeakOffset, 0.1f);
        ImGui::DragFloat("Frag Drag", &fragmentDrag, 0.1f);
        ImGui::DragFloat("Frag Freq Mult", &fragmentFrequencyMult, 0.1f);
        ImGui::DragFloat("Frag Amp Mult", &fragmentAmplitudeMult, 0.1f);
        ImGui::DragFloat("Frag Spd Ramp", &fragmentSpeedRamp, 0.1f);
        ImGui::DragFloat("Frag Seed Iter", &fragmentSeedIter, 0.1f);
        ImGui::DragFloat("Frag Height", &fragmentHeight, 0.1f);
	}
}