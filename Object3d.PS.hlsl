#include "Object3d.hlsli"

struct Material {
	float4 color;
};

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
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
	return output;
}