#include "../PostEffect/BloomEffectInfo.hlsli"

//================================================================
// BloomBlurVertical.CS. hlsl - 垂直方向ガウシアンブラー
//================================================================

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
ConstantBuffer<BloomEffectInfo> gBloomEffectInfo : register(b0);

// 事前計算されたガウシアン重み（最大15サンプル）
static const int MAX_RADIUS = 15;

// ガウシアン関数
float Gaussian(float y, float sigma) {
	return exp(-(y * y) / (2.0f * sigma * sigma));
}

[numthreads(1, 256, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {

	// 出力テクスチャのサイズ取得
	uint width, height;
	gOutputTexture.GetDimensions(width, height);
	// 処理中のピクセル
	int2 pixelPos = int2(DTid.xy);

	// 範囲外チェック
	if ( pixelPos.x >= width || pixelPos.y >= height ) {
		return;
	}

	// 半径をsigmaから算出
	int radius = (int)ceil(3.0f * gBloomEffectInfo.sigma);
	radius = min(radius, MAX_RADIUS);

	float3 result = 0.0f;
	float weightSum = 0.0f;

	// 垂直方向のみサンプリング（1Dブラー）
	for ( int y = -radius; y <= radius; ++y ) {
		// サンプリング位置の計算
		int2 samplePos = pixelPos + int2(0, y);
		samplePos.y = clamp(samplePos.y, 0, (int)height - 1);
		// サンプリング
		float4 sampleColor = gInputTexture.Load(int3(samplePos, 0));
		float weight = Gaussian(float(y), gBloomEffectInfo.sigma);
		// 重み付き加算
		result += sampleColor.rgb * weight;
		weightSum += weight;
	}
	// 正規化して出力
	gOutputTexture[pixelPos] = float4(result / weightSum, 1.0f);
}