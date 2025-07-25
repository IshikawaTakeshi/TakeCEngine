#include "PostEffect/FullScreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixcelShaderOutput {
	float4 color : SV_TARGET0;
};

PixcelShaderOutput main(VertexShaderOutput input) {
	PixcelShaderOutput output;
	output.color = gTexture.Sample(gSampler, input.texcoord);
	return output;
}