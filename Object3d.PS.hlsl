#include "Object3d.hlsli"

struct Material {
	float4 color;
	int enableLighting;
};

struct DirectionalLight {
	float4 color; //
	float3 direction; //ライトの向き
	float intensity; //輝度
};

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//並行光源
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
//テクスチャ
Texture2D<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;
	float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	output.color = gMaterial.color * textureColor;
	
	//Lightingの計算
	if ( gMaterial.enableLighting != 0 ) { //Lightingする場合
		float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
		output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
	} else { //Lightingしない場合。前回まで同じ計算
		output.color = gMaterial.color * textureColor;
	}
	return output;
}