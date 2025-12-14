#include "Light/DirectionalLight.hlsli"
#include "Light/LightCountData.hlsli"
#include "Light/LightCamera.hlsli"
#include "CameraData.hlsli"

//==============================================================
// ShadowMapEffect.CS.hlsl
//==============================================================

struct ShadowMapEffectInfo {
	bool isActive; // エフェクトの有効無効
	float bias; // シャドウのバイアス値
	float normalBias; // シャドウのノーマルバイアス値
	float pcfRange; // テクセル半径	
};

Texture2D<float4> gInputTexture : register(t0); // 入力テクスチャ
Texture2D<float> gShadowMap : register(t1); // ライトカメラの深度テクスチャ
Texture2D<float3> gSceneDepth : register(t2); // メインカメラの深度テクスチャ
RWTexture2D<float4> gOutputTexture : register(u0); // 出力テクスチャ
SamplerState gSampler : register(s0); // サンプラー

// エフェクト情報
ConstantBuffer<ShadowMapEffectInfo> gShadowMapEffectInfo : register(b0);
//ライトカメラ情報
ConstantBuffer<LightCameraInfo> gLightCameraInfo : register(b2);
//メインカメラ情報
ConstantBuffer<CameraData> gMainCameraInfo : register(b3);


//--------------------------------------------------------------
// ワールド座標復元
//--------------------------------------------------------------
float3 ReconstructWorldPos(float2 uv, float depth01) {
    // depth01 が 0-1 の線形深度を想定。非線形の場合は線形化してから使う。
	float2 ndc_xy = uv * 2.0f - 1.0f;
	ndc_xy.y = -ndc_xy.y;
    
	float4 ndc = float4(ndc_xy, depth01, 1.0f);
	float4 wp = mul(ndc, gMainCameraInfo.viewProjectionInverse);
	return wp.xyz / wp.w;
}

//--------------------------------------------------------------
// PCF シャドウサンプリング（法線バイアスなし）
//--------------------------------------------------------------
float SampleShadowPCF(float4 posLS, float2 texelSize, float bias) {
	float3 proj = posLS.xyz / posLS.w;

    // 0-1 空間外なら影の外（照らされる扱い）
	if ( any(proj < 0) || any(proj > 1) )
		return 1.0;

	int r = (int)ceil(gShadowMapEffectInfo.pcfRange);
	float shadow = 0.0;
	float count = 0.0;

    [loop]
	for ( int y = -r; y <= r; ++y ) {
        [loop]
		for ( int x = -r; x <= r; ++x ) {
			float2 offset = float2(x, y) * texelSize;
			float sampleDepth = gShadowMap.SampleLevel(gSampler, proj.xy + offset, 0).r;
			float compare = proj.z - bias;
			shadow += (compare <= sampleDepth) ? 1.0 : 0.0;
			count += 1.0;
		}
	}
	return shadow / max(count, 1.0);
}


//--------------------------------------------------------------
// main
//--------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint w, h;
	gOutputTexture.GetDimensions(w, h);
	if ( DTid.x >= w || DTid.y >= h )
		return;

	float2 uv = (DTid.xy + 0.5) / float2(w, h);
	float4 baseColor = gInputTexture.Load(int3(DTid.xy, 0));
	float depth01 = gSceneDepth.Load(int3(DTid.xy, 0)).r;

	//エフェクト無効ならそのまま出力
	if ( !gShadowMapEffectInfo.isActive ) {
		gOutputTexture[DTid.xy] = baseColor;
		return;
	}

	//ワールド座標を再構成
	float3 worldPos = ReconstructWorldPos(uv, depth01);

	//ライト空間へ変換
	float4 posLS = mul(float4(worldPos, 1.0), gLightCameraInfo.viewProjection);

	//シャドウマップ解像度からtexelSizeを算出（別解像度ならCPUから渡す）
	uint smW, smH;
	gShadowMap.GetDimensions(smW, smH);
	float2 texelSize = 1.0 / float2(smW, smH);

	//シャドウ値を取得
	float shadow = SampleShadowPCF(posLS, texelSize, gShadowMapEffectInfo.bias);

	//結果を出力（影部分を暗くする）
	gOutputTexture[DTid.xy] = float4(baseColor.rgb * shadow, baseColor.a);
}