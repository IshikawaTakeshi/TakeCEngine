#include "../PostEffect/FullScreen.hlsli"

struct DistortionInfo {
	float strength;
	float time;
	float2 scrollSpeed;
	float2 noiseScale;
	float2 offset;
	bool enable;
};

Texture2D<float4> gInputTexture : register(t0);
Texture2D<float4> gNoiseTexture : register(t1);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);
ConstantBuffer<DistortionInfo> gDistortionInfo : register(b0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {

	// 無効ならパススルー
	if (gDistortionInfo.enable == false) {
		gOutputTexture[DTid.xy] = gInputTexture[DTid.xy];
		return;
	}

	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);

	// ノイズテクスチャからUVオフセットを取得
	float2 noiseUV = uv * gDistortionInfo.noiseScale + gDistortionInfo.scrollSpeed * gDistortionInfo.time;
	
	// ノイズ値から指定されたオフセットを引くことでバイアスを解消する
	// 通常の0-1テクスチャなら offset=0.5 で中心付近をゼロにできる
	float2 noiseVal = gNoiseTexture.Sample(gSampler, noiseUV).rg - gDistortionInfo.offset;

	// UV座標をノイズでずらす
	float2 distortedUV = uv + noiseVal * gDistortionInfo.strength;
	// 画面外に飛ばないようクランプ
	distortedUV = saturate(distortedUV);

	float3 resultColor = gInputTexture.Sample(gSampler, distortedUV).rgb;
	gOutputTexture[DTid.xy] = float4(resultColor, 1.0f);
}
