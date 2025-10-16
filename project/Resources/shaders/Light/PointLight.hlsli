#include "../Material.hlsli"

struct PointLight {
	float4 color; //ライトのカラー	
	float3 position; //ライトの位置
	float intensity; //輝度
	float radius; //ライトの届く最大距離
	float decay; //減衰率
};

float3 CalcPointLighting(PointLight pLight, Material gMaterial, float3 N, float3 worldPos, float3 viewDir, float3 albedo) {
	N = normalize(N);
	float3 dir = normalize(pLight.position - worldPos);
	float cosPoint = pow(dot(N, dir) * 0.5f + 0.5f, 2.0f);

	float3 halfVec = normalize(dir + viewDir);
	float NdotH = dot(N, halfVec);
	float spec = pow(saturate(NdotH), gMaterial.shininess);

	float distancePoint = length(pLight.position - worldPos);
	float attenuation = pow(saturate(-distancePoint / pLight.radius + 1.0f), pLight.decay);

	float3 diffuse = albedo * pLight.color.rgb * cosPoint * pLight.intensity * attenuation;
	float3 specular = pLight.color.rgb * pLight.intensity * attenuation * spec;
	return diffuse + specular;
}