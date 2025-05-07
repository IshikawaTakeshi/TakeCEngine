#include "FullScreen.hlsli"

struct GrayScaleType {
	int type;
};

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
ConstantBuffer<GrayScaleType> gType : register(b0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	
	float4 color = gInputTexture[DTid.xy];
	float gray = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
	
	int type = gType.type;
	if (type == 0) {
		gOutputTexture[DTid.xy] = float4(gray, gray, gray, color.a);
	} else if (type == 1) {
		
		float3 sepiaR = gray * float3(1.0f, 0.69f, 0.40f);
		gOutputTexture[DTid.xy] = float4(sepiaR.r,sepiaR.g,sepiaR.b, color.a);
	} else {
		
		gOutputTexture[DTid.xy] = color;
	}
	
}