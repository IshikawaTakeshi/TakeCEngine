#include "../PostEffect/BloomEffectInfo.hlsli"

//================================================================
// BloomEffect.CS.hlsl
//================================================================

Texture2D<float4> gInputTexture : register(t0);
Texture2D<float4> gBloomTexture : register(t1);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);
ConstantBuffer<BloomEffectInfo> gBloomEffectInfo : register(b0);

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	
	// 出力テクスチャのサイズ取得
	uint width, height;
	gOutputTexture.GetDimensions(width, height);
	// 処理中のピクセル
	int2 pixelPos = int2(DTid.xy);

	// 範囲外チェック
	if ( pixelPos.x >= width || pixelPos.y >= height ) {
		return;
	}

	// 元のシーンカラーを取得
	float4 originalColor = gInputTexture.Load(int3(pixelPos, 0));

	// ブルームエフェクトが無効なら元の色を出力して終了
	if ( !gBloomEffectInfo.isActive ) {
		gOutputTexture[pixelPos] = originalColor;
		return;
	}

	// ブルームテクスチャをアップサンプリング（バイリニア補間）
	float2 uv = (float2(pixelPos) + 0.5f) / float2(width, height);
	float4 bloomColor = gBloomTexture.SampleLevel(gSampler, uv, 0);

	// 合成
	float3 finalColor = originalColor.rgb + bloomColor.rgb * gBloomEffectInfo.strength;
	
	gOutputTexture[pixelPos] = float4(finalColor, originalColor.a);
}