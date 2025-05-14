#include "PostEffect/FullScreen.hlsli"

struct DissolveInfo {
	float threshold; //閾値
};

Texture2D<float4> gInputTexture : register(t0);
Texture2D<float4> gMaskTexture  : register(t1);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);
ConstantBuffer<DissolveInfo> gDissolveInfo : register(b0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	
	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float3 resultColor = float3(0.0f, 0.0f, 0.0f);
	
	float mask = gMaskTexture.Sample(gSampler, uv).r;
	
	//maskの値が0.5f以下の場合はdiscard
	if (mask <= gDissolveInfo.threshold) {
		gOutputTexture[DTid.xy] = float4(0.0f, 1.0f, 0.0f, 1.0f);
		return;
	}
	
	resultColor = gInputTexture.Sample(gSampler, uv).rgb;
	gOutputTexture[DTid.xy] = float4(resultColor, 1.0f);
}