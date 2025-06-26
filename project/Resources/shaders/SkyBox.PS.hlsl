#include "SkyBox.hlsli"

struct Material {
	float4 color; //カラー
	float4x4 uvTransform;
	int enableLighting; //Lightingを有効にするフラグ
	float shininess;
};

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//テクスチャ
TextureCube<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

float3 ACESFilm(float3 x) {
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input){
	PixelShaderOutPut output;
	float4 textureColor = gTexture.Sample(gSampler,input.texcoord);
	textureColor.rgb = ACESFilm(textureColor.rgb);
		output.color = textureColor * gMaterial.color;
		return output;
}