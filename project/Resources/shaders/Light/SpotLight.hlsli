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
	Material gMaterial,
	uint lightCount,
	float3 N, float3 worldPos, float3 viewDir, float3 albedo) {
	
	float3 totalLighting = float3(0, 0, 0);
	
	for ( int i = 0; i < lightCount; i++ ) {
		SpotLight sLight = spotLights[i];

		N = normalize(N);
		float3 L = normalize(sLight.position - worldPos);
        
        // diffuse
		float cosSpot = saturate(dot(N, L));

        // specular
		float3 halfVec = normalize(L + viewDir);
		float spec = pow(saturate(dot(N, halfVec)), gMaterial.shininess);

		// 光の減衰計算
		float distanceSpot = length(sLight.position - worldPos);
		
		// 減衰率が0の場合は一定、そうでない場合は減衰計算
		float baseAtten = saturate(1.0f - distanceSpot / sLight.distance);
		float attenuation = lerp(1.0f, pow(baseAtten, sLight.decay), step(0.001f, sLight.decay));

		//スポットライトのコーン判定
		float3 lightDir = normalize(sLight.direction);
		float spotEffect = dot(lightDir, -L);

		float spotAttenuation =
			smoothstep(sLight.cosAngle - sLight.penumbraAngle,
						sLight.cosAngle,
						spotEffect);

		// 最終的な色の計算
		float3 diffuse = albedo * sLight.color.rgb * cosSpot * sLight.intensity * attenuation * spotAttenuation;
		float3 specular = sLight.color.rgb * sLight.intensity * attenuation * spotAttenuation * spec;

		// 合計ライティングに加算
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
	for ( int i = 0; i < lightCount; i++ ) {
		SpotLight sLight = spotLights[i];
		
		// 法線ベクトルを正規化
		N = normalize(N);
		
		// ライト方向ベクトルの計算
		float3 L = normalize(sLight.position - worldPos);
		float cosSpot = saturate(dot(N, L));
		// 光の減衰計算
		float distanceSpot = length(sLight.position - worldPos);
		
		// 減衰率が0の場合は一定、そうでない場合は減衰計算
		float baseAtten = saturate(1.0f - distanceSpot / sLight.distance);
		float attenuation = lerp(1.0f, pow(baseAtten, sLight.decay), step(0.001f, sLight.decay));
		//スポットライトのコーン判定
		float3 lightDir = normalize(sLight.direction);
		float spotEffect = dot(lightDir, -L);
		float spotAttenuation =
			smoothstep(sLight.cosAngle - sLight.penumbraAngle,
						sLight.cosAngle,
						spotEffect);
		
		// 最終的な色の計算
		float3 diffuse = sLight.color.rgb * cosSpot * sLight.intensity * attenuation * spotAttenuation;
		
		// 合計ライティングに加算
		totalLighting += diffuse;
	}
	return totalLighting;
}