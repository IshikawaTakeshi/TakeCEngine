#include "../PostEffect/FullScreen.hlsli"

struct RadialBlurInfo {
	float2 center; 
	float kBlurWidth;
	bool enable;
};

Texture2D<float4> gInputRadialBlurTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

ConstantBuffer<RadialBlurInfo> gBlurInfo : register(b0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	float width, height;
	gInputRadialBlurTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float3 resultColor = float3(0.0f, 0.0f, 0.0f);
	
	
	//サンプリング数。多いほど滑らかになるが重くなる
	const int kNumSamples = 16;
	
	if (gBlurInfo.enable == false) {
		gOutputTexture[DTid.xy] = gInputRadialBlurTexture[DTid.xy];
		return;
	};
	//中心から現在のUVに対しての方向を計算
	float2 direction = normalize(uv - gBlurInfo.center);
	float distance = length(uv - gBlurInfo.center);
	
	for (int sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex) {
		
		float t = sampleIndex / float(kNumSamples - 1); //0から1の範囲でサンプリング位置を計算
		
		//サンプリングのUV座標を計算
		float2 sampleUV = uv + direction * gBlurInfo.kBlurWidth * t * distance;
		//色を加算
		resultColor += gInputRadialBlurTexture.Sample(gSampler, sampleUV).rgb;
	}
	
	//平均化
	resultColor *= rcp(float(kNumSamples));
	
	gOutputTexture[DTid.xy] = float4(resultColor, 1.0f);
}