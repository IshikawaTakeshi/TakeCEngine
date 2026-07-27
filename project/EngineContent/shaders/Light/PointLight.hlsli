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
	uint enabled; //有効フラグ
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
	// 法線ベクトルを正規化
	N = normalize(N);
	// 全てのライトをループ
	for (uint i = 0; i < lightCount; i++) {
		PointLight pLight = pointLights[i];
		
		if( pLight.enabled == 0 ) {
			return totalLighting;
		}
		
		//距離を先に計算して、範囲外なら早期スキップ
		float3 toLight = pLight.position - worldPos;
		float distanceSq = dot(toLight, toLight); // length()より軽い
		float radiusSq = pLight.radius * pLight.radius;
    
		if ( distanceSq > radiusSq ) {
			continue; // 範囲外はスキップ
		}
		
		float distancePoint = sqrt(distanceSq);
		float3 dir = toLight / distancePoint; // normalize()の代わり

		float cosPoint = pow(dot(N, dir) * 0.5f + 0.5f, 2.0f);
	
		// スペキュラ計算
		float3 halfVec = normalize(dir + viewDir);
		float NdotH = dot(N, halfVec);
		float spec = pow(saturate(NdotH), gMaterial.shininess);

		//decayが常に使われる前提で簡略化
		float attenuation = pow(saturate(1.0f - distancePoint / pLight.radius), pLight.decay);

		// 最終的な色の計算
		float3 diffuse = albedo * pLight.color.rgb * cosPoint * pLight.intensity * attenuation;
		float3 specular = pLight.color.rgb * pLight.intensity * attenuation * spec;

		totalLighting += diffuse + specular;
	}

	return totalLighting;
}

//===============================
//ポイントライトの計算（シンプル版）
//===============================
float3 CalcPointLightingSimple(
	StructuredBuffer<PointLight> pointLights,
	uint lightCount,
	float3 N, float3 worldPos) {
	float3 totalLighting = float3(0, 0, 0);
	N = normalize(N);
	// 全てのライトをループ
	for (uint i = 0; i < lightCount; i++) {
		PointLight pLight = pointLights[i];
		
		//距離を先に計算して、範囲外なら早期スキップ
		float3 toLight = pLight.position - worldPos;
		float distanceSq = dot(toLight, toLight); // length()より軽い
		float radiusSq = pLight.radius * pLight.radius;
    
		if ( distanceSq > radiusSq ) {
			continue; // 範囲外はスキップ
		}
		
		float distancePoint = sqrt(distanceSq);
		float3 dir = toLight / distancePoint;
		float cosPoint = pow(dot(N, dir) * 0.5f + 0.5f, 2.0f);
		// 光の減衰計算
		//decayが常に使われる前提
		float attenuation = pow(saturate(1.0f - distancePoint / pLight.radius), pLight.decay);
		float3 diffuse = pLight.color.rgb * cosPoint * pLight.intensity * attenuation;
		totalLighting += diffuse;
	}
	return totalLighting;
}