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
	N = normalize(N);
	float3 sdir = normalize(sLight.position - worldPos);

	float distanceSpot = length(sLight.position - worldPos);
	float attenuation = pow(saturate(-distanceSpot / sLight.distance + 1.0f), sLight.decay);

	float cosAngle = dot(sdir, sLight.direction);
	float falloff = saturate((cosAngle - sLight.cosAngle) / (sLight.penumbraAngle - sLight.cosAngle));

	float3 halfVec = normalize(sdir + viewDir);
	float NdotH = dot(N, halfVec);
	float spec = pow(saturate(NdotH), gMaterial.shininess);

	float3 diffuse = albedo * sLight.color.rgb * sLight.intensity * attenuation * falloff;
	float3 specular = sLight.color.rgb * sLight.intensity * attenuation * falloff * spec;
	return diffuse + specular;
}