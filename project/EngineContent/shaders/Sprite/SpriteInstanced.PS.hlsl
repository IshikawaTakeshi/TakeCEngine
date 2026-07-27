#include "SpriteInstanced.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;
	float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	output.color = input.color * textureColor;
	return output;
}
