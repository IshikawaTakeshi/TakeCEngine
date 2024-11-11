#include "Object3d.hlsli"

struct Material {
	float4 color; //カラー
	float4x4 uvTransform;
	int enableLighting; //Lightingを有効にするフラグ
	float shininess;
};

struct DirectionalLight {
	float4 color; //ライトのカラー	
	float3 direction; //ライトの向き
	float intensity; //輝度
};

struct Camera {
	float3 worldPosition;
};

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//並行光源
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
//カメラ
ConstantBuffer<Camera> gCamera : register(b2);
//テクスチャ
Texture2D<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;

	float4 transformedUV = mul(float4(input.texcoord,0.0f,1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	
	if (textureColor.a < 0.5f) { discard; }

	if (output.color.a == 0.0f) { discard; }

	//Lightingの計算
	if (gMaterial.enableLighting != 0) { //Lightingする場合
	
		float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction); //法線とライト方向の内積
		float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
		float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
		//float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
		//float RdotE = dot(reflectLight, toEye);
		float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
		float NdotH = dot(normalize(input.normal), halfVector);
		float specularPow = pow(saturate(NdotH), gMaterial.shininess);
	
		//拡散反射
		float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
		//鏡面反射
		float3 specular = gMaterial.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
		//拡散反射+鏡面反射
		output.color.rgb = diffuse + specular;
		//アルファ値
		output.color.a = gMaterial.color.a * textureColor.a;
		//	//ランバート反射
		//	float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
		//	output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;

	} else { //Lightingしない場合。前回まで同じ計算
		output.color = gMaterial.color * textureColor;
	}
	
	return output;
}