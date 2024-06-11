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
//テクスチャ
Texture2D<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);
//並行光源
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);


struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;
	float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	output.color = gMaterial.color * textureColor;
	return output;
}