#include "../PostEffect/BloomEffectInfo.hlsli"


//================================================================
// BloomBrightPass.CS.hlsl - 高輝度部分の抽出
//================================================================

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
ConstantBuffer<BloomEffectInfo> gBloomEffectInfo : register(b0);

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gOutputTexture.GetDimensions(width, height);

	int2 pixelPos = int2(DTid.xy);

	if ( pixelPos.x >= width || pixelPos.y >= height ) {
		return;
	}

	// 縮小解像度でサンプリング（4ピクセルの平均）
	int2 srcPos = pixelPos * 2;
	float4 color = gInputTexture.Load(int3(srcPos, 0));
	color += gInputTexture.Load(int3(srcPos + int2(1, 0), 0));
	color += gInputTexture.Load(int3(srcPos + int2(0, 1), 0));
	color += gInputTexture.Load(int3(srcPos + int2(1, 1), 0));
	color *= 0.25f;

	// 輝度計算と閾値処理（ここで1回だけ実行）
	float luminance = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
	float bright = smoothstep(gBloomEffectInfo.threshold, gBloomEffectInfo.threshold + 0.1f, luminance);

	// 高輝度部分のみ出力
	gOutputTexture[pixelPos] = float4(color.rgb * bright, 1.0f);
}