#include "FullScreen.hlsli"

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	float4 color = gInputTexture[DTid.xy];
	float gray = dot(color.rgb, float3(0.2125f,0.7154f, 0.0721f));
	gOutputTexture[DTid.xy] = float4(gray, gray, gray, color.a);
}