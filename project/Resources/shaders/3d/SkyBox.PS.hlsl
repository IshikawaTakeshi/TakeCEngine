#include "SkyBox.hlsli"
#include "../Material.hlsli"

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//テクスチャ
TextureCube<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
	float depth : SV_DEPTH;
};

PixelShaderOutPut main(VertexShaderOutput input){
	PixelShaderOutPut output;
	float4 textureColor = gTexture.Sample(gSampler,input.texcoord);
	output.color = textureColor * gMaterial.color;
	output.depth = 1.0f;
	return output;
}