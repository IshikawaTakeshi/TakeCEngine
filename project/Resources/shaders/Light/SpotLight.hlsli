#pragma once
#include "../Material.hlsli"

struct SpotLight {
	float4 color; //ライトのカラー	
	float3 position; //ライトの位置
	float intensity; //輝度
	float3 direction; //ライトの向き
	float distance; //ライトの届く最大距離
	float decay; //減衰率
	float cosAngle; //スポットライトの角度
	float penumbraAngle; //影のぼかし角度
	float padding; //パディング
};

float3 CalcSpotLighting(
	StructuredBuffer<SpotLight> spotLights,
	Material gMaterial,
	uint lightCount,
	float3 N, float3 worldPos, float3 viewDir, float3 albedo) {
	
	float3 totalLighting = float3(0, 0, 0);
	
	//全てのライトをループ
	for(int i = 0; i < lightCount; i++) {
		SpotLight sLight = spotLights[i];
		
		//法線ベクトルを正規化
		N = normalize(N);
		float3 dir = normalize(sLight.position - worldPos);
		float cosSpot = pow(dot(N, dir) * 0.5f + 0.5f, 2.0f);
		
		//スペキュラ計算
		float3 halfVec = normalize(dir + viewDir);
		float NdotH = dot(N, halfVec);
		float spec = pow(saturate(NdotH), gMaterial.shininess);
		
		//光の減衰計算
		float distanceSpot = length(sLight.position - worldPos);
		float attenuation = pow(saturate(-distanceSpot / sLight.distance + 1.0f), sLight.decay);
		
		//スポットライトのコーン内かどうか
		float3 lightDir = normalize(-sLight.direction);
		float spotEffect = dot(dir, lightDir);
		
		float spotAttenuation = smoothstep(sLight.cosAngle, sLight.cosAngle - sLight.penumbraAngle, spotEffect);
		
		//最終的な色の計算
		float3 diffuse = albedo * sLight.color.rgb * cosSpot * sLight.intensity * attenuation * spotAttenuation;
		float3 specular = sLight.color.rgb * sLight.intensity * attenuation * spotAttenuation * spec;
		
		totalLighting += diffuse + specular;
	}
	
	return totalLighting;
}