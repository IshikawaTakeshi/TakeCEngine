//Lighting.hlsli
#include "Light/DirectionalLight.hlsli"
#include "Light/PointLight.hlsli"
#include "Light/SpotLight.hlsli"

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
StructuredBuffer<PointLight> gPointLights : register(t2);
StructuredBuffer<SpotLight> gSpotLights : register(t3);

float3 ComputeLighting(float3 normal, float3 worldPos, float3 viewDir, float3 albedo) {
	float3 result = float3(0.0f, 0.0f, 0.0f);
	// 並行光源の影響を計算
	result += CalcDirectionalLighting(gDirectionalLight, gMaterial, normal, viewDir, albedo);

	// Point lights
	for (uint i = 0; i < gNumPointLights; ++i) {
		result += CalcPointLighting(gPointLights[i], gMaterial, normal, worldPos, viewDir, albedo);
	}
	// Spot lights
	for (uint i = 0; i < gNumSpotLights; ++i) {
		result += CalcSpotLighting(gSpotLights[i], gMaterial, normal, worldPos, viewDir, albedo);
	}
	return result;
}

