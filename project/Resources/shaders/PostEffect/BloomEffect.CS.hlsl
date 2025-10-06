
//================================================================
// BloomEffect.CS.hlsl
//================================================================

struct BloomEffectInfo {
	bool isActive;
	float threshold;
	float strength;
	float sigma;
};

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

ConstantBuffer<BloomEffectInfo> gBloomEffectInfo : register(b0);

float Gaussian(float x, float s) {
	return exp(-(x * x) / (2.0f * s * s));

}

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);
	
	//処理中のピクセル
	int2 pixelPos = int2(DTid.xy);
	
	//元カラー
	float4 originalColor = gInputTexture.Load(int3(pixelPos, 0));
	
	//範囲外だった場合
	if(pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	//無効化されている場合は元の色をそのまま出力
	if(gBloomEffectInfo.isActive == false) {
		gOutputTexture[pixelPos] = originalColor;
		return;
	}
	
	// 半径をsigmaから算出
	int radius = (int)ceil(3.0f * gBloomEffectInfo.sigma);
	
	//サンプリング
	float3 bloomAccum = 0.0f;
	float weightSum = 0.0f;
	
	for ( int y = -radius; y <= radius; ++y ) {
		for ( int x = -radius; x <= radius; ++x ) {
			
			int2 samplePos = pixelPos + int2(x, y);
			
			//範囲外だった場合はスキップ
			if(samplePos.x < 0 || samplePos.y < 0 || samplePos.x >= width || samplePos.y >= height) {
				continue;
			}
			
			//サンプリング
			float4 sampleColor = gInputTexture.Load(int3(samplePos, 0));
			
			//閾値以上の色のみを対象にする
			float luminance = dot(sampleColor.rgb, float3(0.2126f, 0.7152f, 0.0722f));
			float lum = luminance;
			float bright = max(lum - gBloomEffectInfo.threshold, 0.0); // 超過分
			// ソフト閾値（k = 0.1f 等）滑らかステップ
			bright = smoothstep(gBloomEffectInfo.threshold, gBloomEffectInfo.threshold + 0.1f, lum);
			
			//ガウシアン重み
			float weight = Gaussian(length(float2(x, y)), gBloomEffectInfo.sigma);
			
			bloomAccum += sampleColor.rgb * bright * weight;
			weightSum += bright * weight;
		}
	}
	
	//合成処理
	float3 bloomColor = (weightSum > 0.0f) ? bloomAccum / weightSum : 0.0f;
	float3 finalColor = originalColor.rgb + bloomColor * gBloomEffectInfo.strength;
	gOutputTexture[pixelPos] = float4(finalColor, originalColor.a);
}