#include "FullScreen.hlsli"


Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

static const float2 kIndex3x3[3][3] = {
	{ { -1.0f, 1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
	{ { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -1.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};

static const float kKernel3x3[3][3] = {
	{ 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
	{ 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
	{ 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
};



[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	
	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float2 uvStepSize = float2(rcp(width), rcp(height));
	float4 resultColor;
	
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) { // 3x3 kernel
			
			//現在のtextureのUV座標を取得
			float2 offset = kIndex3x3[x][y] * uvStepSize;
			float2 sampleUV = uv + offset;
			
			//色に1/9を掛ける
			resultColor.rgb = gInputTexture.Sample(gSampler, sampleUV).rgb;
			gOutputTexture[DTid.xy].rgb += resultColor.rgb * kKernel3x3[x][y];
		}
	}

	gOutputTexture[DTid.xy] = float4(gOutputTexture[DTid.xy].rgb, 1.0f);
}