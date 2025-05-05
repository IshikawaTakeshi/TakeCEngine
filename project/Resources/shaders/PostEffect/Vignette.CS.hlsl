#include "Fullscreen.hlsli"

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	
	float4 color = gInputTexture[DTid.xy];
	
	//周囲を0,中心になるほど明るくなるように計算で調整
	float2 correct = DTid.xy * (1.0f - DTid.yx);
	
	float vignette = correct.x * correct.y * 16.0f;
	
	vignette = saturate(pow(vignette, 0.8f));
	
	gOutputTexture[DTid.xy] *= vignette;
}