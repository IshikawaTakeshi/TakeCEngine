#pragma once
#include "../Material.hlsli"

//===============================
//ポイントライト
//===============================

struct PointLight {
	float4 color; //ライトのカラー	
	float3 position; //ライトの位置
	float intensity; //輝度
	float radius; //ライトの届く最大距離
	float decay; //減衰率
};

//===============================
//ポイントライトの計算
//===============================

float3 CalcPointLighting(
	StructuredBuffer<PointLight> pointLights,
	Material gMaterial,
	uint lightCount,
	float3 N,float3 worldPos, float3 viewDir, float3 albedo) {
	
	float3 totalLighting = float3(0, 0, 0);
    
    // 全てのライトをループ
	for (uint i = 0; i < lightCount; i++) {
		PointLight pLight = pointLights[i];
        
        // 法線ベクトルを正規化
		N = normalize(N);
		float3 dir = normalize(pLight.position - worldPos);
		float cosPoint = pow(dot(N, dir) * 0.5f + 0.5f, 2.0f);
        
        // スペキュラ計算
		float3 halfVec = normalize(dir + viewDir);
		float NdotH = dot(N, halfVec);
		float spec = pow(saturate(NdotH), gMaterial.shininess);
        
        // 光の減衰計算
		float distancePoint = length(pLight.position - worldPos);
		float attenuation = pow(saturate(-distancePoint / pLight.radius + 1.0f), pLight.decay);
        
        // 最終的な色の計算
		float3 diffuse = albedo * pLight.color.rgb * cosPoint * pLight.intensity * attenuation;
		float3 specular = pLight.color.rgb * pLight.intensity * attenuation * spec;
        
		totalLighting += diffuse + specular;
	}
    
	return totalLighting;
}