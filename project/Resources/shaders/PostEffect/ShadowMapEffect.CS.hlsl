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
Texture2D<float> gShadowMapDepth : register(t1); // ライトカメラの深度テクスチャ
Texture2D<float> gSceneDepth : register(t2); // メインカメラの深度テクスチャ
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

float SampleShadowPCF(float4 posLS, float bias, float pcfRange) {
	float3 proj = posLS.xyz / posLS.w;
	float2 shadowUV = proj.xy * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
    
    // 範囲外チェック
	if ( shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f || shadowUV.y > 1.0f ||
        proj.z < 0.0f || proj.z > 1.0f ) {
		return 1.0f;
	}
    
	float currentDepth = proj.z - bias;
    
    // シャドウマップの解像度を取得
	uint w, h;
	gShadowMapDepth.GetDimensions(w, h);
	float2 texelSize = 1.0f / float2(w, h);
    
    // 3x3 PCFサンプリング
	float shadow = 0.0f;
	int sampleCount = 0;
    
	for ( int y = -1; y <= 1; y++ ) {
		for ( int x = -1; x <= 1; x++ ) {
			float2 offset = float2(x, y) * texelSize * pcfRange;
			float depth = gShadowMapDepth.SampleLevel(gSampler, shadowUV + offset, 0).r;
			shadow += (currentDepth <= depth) ? 1.0f : 0.0f;
			sampleCount++;
		}
	}
    
	return shadow / float(sampleCount); // 0.0〜1.0 の滑らかな値
}

//--------------------------------------------------------------
// シャドウ判定（PCF なし）
//--------------------------------------------------------------
float SampleShadow(float4 posLS, float bias) {
    // 同次座標を正規化
	float3 proj = posLS.xyz / posLS.w;
    
    // NDC → UV 座標（0〜1）に変換
	float2 shadowUV = proj.xy * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y; // Y 反転
    
    // 範囲外チェック
	if ( shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f || shadowUV.y > 1.0f ||
        proj.z < 0.0f || proj.z > 1.0f ) {
		return 1.0f; // 範囲外は影なし
	}
    
    // シャドウマップから深度をサンプル
	float shadowDepth = gShadowMapDepth.SampleLevel(gSampler, shadowUV, 0).r;
    
    // 深度比較
	float currentDepth = proj.z - bias;
    
    // 影の中なら 0、そうでなければ 1
	return (currentDepth <= shadowDepth) ? 1.0f : 0.0f;
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
    
    // UV 座標
	float2 uv = (DTid.xy + 0.5f) / float2(w, h);
    
    // 入力カラー
	float4 baseColor = gInputTexture.Load(int3(DTid.xy, 0));
    
    // メインカメラの
	float depth = gSceneDepth.Load(int3(DTid.xy, 0)).r;
    
    // エフェクト無効ならそのまま出力
	if ( !gShadowMapEffectInfo.isActive ) {
		gOutputTexture[DTid.xy] = baseColor;
		return;
	}
    
    // スカイボックス等（深度が遠い）はスキップ
	if ( depth >= 0.9999f ) {
		gOutputTexture[DTid.xy] = baseColor;
		return;
	}
    
    // ワールド座標を再構成
	float3 worldPos = ReconstructWorldPos(uv, depth);
    
    // ライト空間へ変換
	float4 posLS = mul(float4(worldPos, 1.0f), gLightCameraInfo.viewProjection);
    
    // シャドウ判定
	float shadow = SampleShadow(posLS, gShadowMapEffectInfo.bias);
    
    // 結果を出力
	gOutputTexture[DTid.xy] = float4(baseColor.rgb * shadow, baseColor.a);
	
// 【デバッグ3】ライト空間UVの可視化
// これで「ライトが当たっているはずの範囲」がカラフルに表示されます。

//	float3 proj = posLS.xyz / posLS.w;
//	float2 shadowUV = proj.xy * 0.5f + 0.5f;
//	shadowUV.y = 1.0f - shadowUV.y;

//// 範囲外（ライトカメラの撮影範囲外）なら「赤」を表示
//	if ( shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
//    shadowUV.y < 0.0f || shadowUV.y > 1.0f ||
//    proj.z < 0.0f || proj.z > 1.0f ) {
//    // 赤色 = 影の計算範囲外（設定したWidth/HeightやNear/Farの外側）
//		gOutputTexture[DTid.xy] = float4(1.0f, 0.0f, 0.0f, 1.0f);
//	}
//	else {
//    // 緑〜黄色グラデーション = 影の計算範囲内（正常）
//    // UV座標を色として出す
//		gOutputTexture[DTid.xy] = float4(shadowUV.x, shadowUV.y, 0.0f, 1.0f);
//	}
//	return;
}