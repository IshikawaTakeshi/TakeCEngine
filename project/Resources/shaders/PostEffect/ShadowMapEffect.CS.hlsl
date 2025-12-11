#include "../Light/SpotLight.hlsli"
#include "../Light/LightCountData.hlsli"
#include "../CameraData.hlsli"

//==============================================================
// ShadowMapEffect.CS.hlsl
//==============================================================

struct ShadowMapEffectInfo {
	bool isActive; // エフェクトの有効無効
	float bias; // シャドウのバイアス値
	float normalBiasf; // シャドウのノーマルバイアス値
	float pcfRange; // テクセル半径	
};

Texture2D<float4> gInputTexture : register(t0); 　　// 入力テクスチャ
Texture2D<float> gDepthTexture : register(t1);     // 深度テクスチャ
Texture2D<float3> gNormalTexture : register(t2);   // 法線テクスチャ
StructuredBuffer<SpotLight> gSpotLight : register(t3); //スポットライト
RWTexture2D<float4> gOutputTexture : register(u0); // 出力テクスチャ
SamplerState gSampler : register(s0);

// エフェクト情報
ConstantBuffer<ShadowMapEffectInfo> gShadowMapEffectInfo : register(b0);
// ライト数カウンター
ConstantBuffer<LightCountData> gLightCount : register(b1);
//カメラ
ConstantBuffer<CameraData> gCamera : register(b2);

float3 ReconstructWorldPos(float2 uv, float depth01) {
    // depth01 が 0-1 の線形深度を想定。非線形の場合は線形化してから使う。
	float4 ndc = float4(uv * 2.0f - 1.0f, depth01, 1.0f);
	float4 wp = mul(ndc, );
	return wp.xyz / wp.w;
}

float SampleShadowPCF(float4 posLS, float2 texelSize, float bias, float normalBias, float pcfRange) {
	float3 proj = posLS.xyz / posLS.w;
	if (proj.x < 0 || proj.x > 1 || proj.y < 0 || proj.y > 1 || proj.z < 0 || proj.z > 1)
		return 1.0f;

	int radius = (int) ceil(pcfRange); // pcfRange=1 → 3x3 相当
	float shadow = 0.0f;
	float count = 0.0f;

    [loop]
	for (int y = -radius; y <= radius; ++y) {
        [loop]
		for (int x = -radius; x <= radius; ++x) {
			float2 o = float2(x, y) * texelSize;
			float sampleDepth = gDepthTexture.SampleLevel(gSampler, proj.xy + o, 0).r;
			float compareDepth = proj.z - bias - normalBias;
			shadow += (compareDepth <= sampleDepth) ? 1.0f : 0.0f;
			count += 1.0f;
		}
	}
	return shadow / max(count, 1.0f);
}

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	
	// 出力テクスチャのサイズ取得
	uint width, height;
	gOutputTexture.GetDimensions(width, height);
	// 処理中のピクセル
	int2 pixelPos = int2(DTid.xy);

	// 範囲外チェック
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}

	// 元のシーンカラーを取得
	float4 originalColor = gInputTexture.Load(int3(pixelPos, 0));
	
	
}