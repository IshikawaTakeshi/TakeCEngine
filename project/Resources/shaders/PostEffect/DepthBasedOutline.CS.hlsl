#include "PostEffect/Fullscreen.hlsli"
#include "PostEffect/Outline.hlsli"
#include "CameraData.hlsli"

struct DepthBasedOutlineInfo {
	float4 Color;
	float weight;
	float distantSensitivity; //遠方オブジェクトの感度調整係数
	float distantStart; //遠方補正を始めるviewZ
	float distantEnd; //補正を最大にするviewZ
	bool isActive;
};

Texture2D<float4> gInputTexture : register(t0);
Texture2D<float> gDepthTexture : register(t1);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);
SamplerState gSamplerPoint : register(s1);

ConstantBuffer<DepthBasedOutlineInfo> gOutlineInfo : register(b0);
ConstantBuffer<CameraData> gCameraInfo : register(b1);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	
	if (gOutlineInfo.isActive == false) {
		gOutputTexture[DTid.xy] = gInputTexture[DTid.xy];
		return;
	}
	
	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float2 uvStepSize = float2(rcp(width), rcp(height));
	float3 resultColor = float3(0.0f, 0.0f, 0.0f);
	float2 difference = { 0.0f, 0.0f };
	// 畳み込み
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) { // 3x3 kernel
			// 現在のtextureのUV座標を取得
			float2 offset = kIndex3x3[x][y] * uvStepSize;
			float2 sampleUV = uv + offset;
			sampleUV = clamp(sampleUV, float2(0.0, 0.0), float2(1.0, 1.0));
			//ndc -> view
			float ndcDepth = gDepthTexture.Sample(gSamplerPoint, sampleUV);
			float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gCameraInfo.projectionInverse);
			float viewZ = viewSpace.z * rcp(viewSpace.w);
			float distantLerp = saturate((abs(viewZ) - gOutlineInfo.distantStart) / (gOutlineInfo.distantEnd - gOutlineInfo.distantStart));
			float localWeight = lerp(1.0, gOutlineInfo.distantSensitivity, distantLerp);

			difference.x += viewZ * kPrewittHorizontalKernel[x][y] * localWeight;
			difference.y += viewZ * kPrewittVerticalKernel[x][y] * localWeight;
		}
	}
	float weight = length(difference);
	weight = saturate(weight * gOutlineInfo.weight);
	resultColor = lerp(gInputTexture.Sample(gSampler, uv).rgb, gOutlineInfo.Color.rgb, weight);
	gOutputTexture[DTid.xy] = float4(resultColor, 1.0f);
}