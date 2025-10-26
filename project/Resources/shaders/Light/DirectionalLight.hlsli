#include "../Material.hlsli"

struct DirectionalLight {
	float4 color;
	float3 direction;
	float intensity;
};

float3 CalcDirectionalLighting(DirectionalLight dLight,Material gMaterial,float3 N, float3 viewDir, float3 albedo) {
	N = normalize(N);
	float NdotL = dot(N, -dLight.direction);
	float cosTerm = pow(NdotL * 0.5f + 0.5f, 2.0f);

	float3 halfVec = normalize(-dLight.direction + viewDir);
	float NdotH = dot(N, halfVec);
	float spec = pow(saturate(NdotH), gMaterial.shininess);

	float3 diffuse = albedo * dLight.color.rgb * cosTerm * dLight.intensity;
	float3 specular = dLight.color.rgb * dLight.intensity * spec;
	return diffuse + specular;
}