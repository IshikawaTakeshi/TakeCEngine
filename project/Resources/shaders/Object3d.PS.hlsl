#include "Object3d.hlsli"
#include "CameraData.hlsli"
#include "Light/DirectionalLight.hlsli"
#include "Light/PointLight.hlsli"
#include "Light/SpotLight.hlsli"
#include "Light/LightCountData.hlsli"

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//並行光源
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
//カメラ
ConstantBuffer<CameraData> gCamera : register(b2);
//ライト数カウンター
ConstantBuffer<LightCountData> gLightCount : register(b3);


//テクスチャ
Texture2D<float4> gTexture : register(t0);
//環境マップ用テクスチャ
TextureCube<float4> gEnvMapTexture : register(t1);

//ポイントライト
StructuredBuffer<PointLight> gPointLight : register(t2);
//スポットライト
StructuredBuffer<SpotLight> gSpotLight : register(t3);
//サンプラー
SamplerState gSampler : register(s0);


PixelShaderOutput main(VertexShaderOutput input) {
	PixelShaderOutput output;
	Material material = gMaterial;
	DirectionalLight dirLightInfo = gDirectionalLight;

	float4 transformedUV = mul(float4(input.texcoord,0.0f,1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	
	if (textureColor.a < 0.5f) { discard; }
	
	//Lightingの計算
	if (gMaterial.enableLighting == 1) {
		
		//DirectionalLightの計算
		float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
		float3 dirLighting = CalcDirectionalLighting(dirLightInfo, material, input.normal, toEye, material.color.rgb * textureColor.rgb);
		
		//pointLightの計算(光の減衰込み)
		float3 pointLighting = CalcPointLighting(
			gPointLight,
			material,
			gLightCount.pointLightCount,
			input.normal, input.worldPosition, toEye, gMaterial.color.rgb * textureColor.rgb);
		
		//spotLightの計算
		float3 spotLighting = CalcSpotLighting(
			gSpotLight, 
			material,
			gLightCount.spotLightCount,
			input.normal, input.worldPosition, toEye, gMaterial.color.rgb * textureColor.rgb);

		//環境マップの計算
		float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
		float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
		float4 environmentColor = gEnvMapTexture.Sample(gSampler, reflectedVector);
		
		//拡散反射+鏡面反射
		output.color.rgb = dirLighting + pointLighting + spotLighting;
		//環境マップ
		output.color.rgb += environmentColor.rgb * gMaterial.envCoefficient;
		//アルファ値
		output.color.a = gMaterial.color.a * textureColor.a;
		
	}else if (gMaterial.enableLighting == 0) { //Lightingしない場合。前回まで同じ計算
		output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
		output.color.a = gMaterial.color.a * textureColor.a;
	}
	
	return output;
}