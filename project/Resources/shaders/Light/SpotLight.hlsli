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

//===============================
//スポットライトの計算
//===============================
float3 CalcSpotLighting(
	StructuredBuffer<SpotLight> spotLights,
	float shininess,
	uint lightCount,
	float3 N, float3 worldPos, float3 viewDir, float3 albedo) {
	
	float3 totalLighting = float3(0, 0, 0);
	
	// ✅ ループ外で正規化（1回だけ）
	N = normalize(N);
	
	for ( uint i = 0; i < lightCount; i++ ) {
		SpotLight sLight = spotLights[i];
		
		// ✅ 早期カリング（距離チェック）を最初に行う
		float3 toLight = sLight.position - worldPos;
		float distanceSq = dot(toLight, toLight);
		float maxDistSq = sLight.distance * sLight.distance;
		
		if ( distanceSq > maxDistSq ) {
			continue; // 範囲外はスキップ
		}
		
		// ✅ toLight を再利用して L を計算
		float distanceSpot = sqrt(distanceSq);
		float3 L = toLight / distanceSpot; // normalize の代わり
		
		// ✅ スポットライトのコーン判定（早期スキップ）
		float3 lightDir = normalize(sLight.direction);
		float spotEffect = dot(lightDir, -L);
		
		// コーンの外側なら早期スキップ
		if ( spotEffect < sLight.cosAngle - sLight.penumbraAngle ) {
			continue;
		}
		
		// diffuse
		float cosSpot = saturate(dot(N, L));
		
		// specular
		float3 halfVec = normalize(L + viewDir);
		float spec = pow(saturate(dot(N, halfVec)), shininess);
		
		// 減衰率が0の場合は一定、そうでない場合は減衰計算
		float baseAtten = saturate(1.0f - distanceSpot / sLight.distance);
		float attenuation = lerp(1.0f, pow(baseAtten, sLight.decay), step(0.001f, sLight.decay));
		
		// スポットライトの減衰
		float spotAttenuation = smoothstep(
			sLight.cosAngle - sLight.penumbraAngle,
			sLight.cosAngle,
			spotEffect);
		
		// ✅ 共通項をまとめる
		float3 lightContrib = sLight.color.rgb * sLight.intensity * attenuation * spotAttenuation;
		
		// 最終的な色の計算
		float3 diffuse = albedo * lightContrib * cosSpot;
		float3 specular = lightContrib * spec;
		
		totalLighting += diffuse + specular;
	}
	
	return totalLighting;
}

//===============================
//スポットライトの計算（シンプル版）
//===============================
float3 CalcSpotLightingSimple(
	StructuredBuffer<SpotLight> spotLights,
	uint lightCount,
	float3 N, float3 worldPos) {
	
	float3 totalLighting = float3(0, 0, 0);
	
	// ✅ ループ外で正規化（1回だけ）
	N = normalize(N);
	
	for ( uint i = 0; i < lightCount; i++ ) {
		SpotLight sLight = spotLights[i];
		
		// ✅ 早期カリング（距離チェック）を最初に行う
		float3 toLight = sLight.position - worldPos;
		float distanceSq = dot(toLight, toLight);
		float maxDistSq = sLight.distance * sLight.distance;
		
		if ( distanceSq > maxDistSq ) {
			continue; // 範囲外はスキップ
		}
		
		// ✅ toLight を再利用して L を計算
		float distanceSpot = sqrt(distanceSq);
		float3 L = toLight / distanceSpot; // normalize の代わり
		
		// ✅ スポットライトのコーン判定（早期スキップ）
		float3 lightDir = normalize(sLight.direction);
		float spotEffect = dot(lightDir, -L);
		
		// コーンの外側なら早期スキップ
		if ( spotEffect < sLight.cosAngle - sLight.penumbraAngle ) {
			continue;
		}
		
		// diffuse
		float cosSpot = saturate(dot(N, L));
		
		// 減衰計算
		float baseAtten = saturate(1.0f - distanceSpot / sLight.distance);
		float attenuation = lerp(1.0f, pow(baseAtten, sLight.decay), step(0.001f, sLight.decay));
		
		// スポットライトの減衰
		float spotAttenuation = smoothstep(
			sLight.cosAngle - sLight.penumbraAngle,
			sLight.cosAngle,
			spotEffect);
		
		// 最終的な色の計算
		float3 diffuse = sLight.color.rgb * cosSpot * sLight.intensity * attenuation * spotAttenuation;
		
		totalLighting += diffuse;
	}
	
	return totalLighting;
}