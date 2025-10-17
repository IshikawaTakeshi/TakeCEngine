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
};

float3 CalcSpotLighting(SpotLight sLight,Material gMaterial,float3 N, float3 worldPos, float3 viewDir, float3 albedo) {
	
	//法線ベクトルを正規化
	N = normalize(N);
	float3 sdir = normalize(sLight.position - worldPos);

	//スポットライトの内側か外側かで処理を分ける
	float distanceSpot = length(sLight.position - worldPos);
	float attenuation = pow(saturate(-distanceSpot / sLight.distance + 1.0f), sLight.decay);

	//falloff計算
	float cosAngle = dot(sdir, sLight.direction);
	float falloff = saturate((cosAngle - sLight.cosAngle) / (sLight.penumbraAngle - sLight.cosAngle));

	//Diffuse計算
	float3 halfVec = normalize(sdir + viewDir);
	float NdotH = dot(N, halfVec);
	float spec = pow(saturate(NdotH), gMaterial.shininess);

	//最終的な色の計算
	float3 diffuse = albedo * sLight.color.rgb * sLight.intensity * attenuation * falloff;
	float3 specular = sLight.color.rgb * sLight.intensity * attenuation * falloff * spec;
	return diffuse + specular;
}