#include "../PostEffect/Fullscreen.hlsli"

struct ChromaticAberrationInfo {
	float2 center;     // 中心位置
	float intensity;   // 強度
	float redScale;    // 赤のズレ倍率
	float greenScale;  // 緑のズレ倍率
	float blueScale;   // 青のズレ倍率
	int isActive;      // 有効無効フラグ
};

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

ConstantBuffer<ChromaticAberrationInfo> gChromaInfo : register(b0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	
	// 無効ならそのままコピーして終了
	if (gChromaInfo.isActive == 0) {
		gOutputTexture[DTid.xy] = gInputTexture[DTid.xy];
		return;
	}

	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	
	// 中心からのオフセットベクトルを計算
	float2 offset = uv - gChromaInfo.center;
	
	// Intensityを加味して、各チャンネルの実際のスケールを算出
	// scaleが1.0であればズレなし。強度によって1.0からの差分を乗算する。
	float rScale = 1.0f + (gChromaInfo.redScale - 1.0f) * gChromaInfo.intensity;
	float gScale = 1.0f + (gChromaInfo.greenScale - 1.0f) * gChromaInfo.intensity;
	float bScale = 1.0f + (gChromaInfo.blueScale - 1.0f) * gChromaInfo.intensity;
	
	// それぞれのUVを計算
	float2 uvR = gChromaInfo.center + offset * rScale;
	float2 uvG = gChromaInfo.center + offset * gScale;
	float2 uvB = gChromaInfo.center + offset * bScale;
	
	// 画面外の場合は、元のUVを使用するようにクランプするか、境界を反映
	// (SampleLevelを使うことでサンプラーの設定通りにクランプ/ラップ処理される)
	float r = gInputTexture.SampleLevel(gSampler, uvR, 0).r;
	float g = gInputTexture.SampleLevel(gSampler, uvG, 0).g;
	float b = gInputTexture.SampleLevel(gSampler, uvB, 0).b;
	
	// アルファは元の座標のままか1.0として出力
	float a = gInputTexture.SampleLevel(gSampler, uv,  0).a;
	
	gOutputTexture[DTid.xy] = float4(r, g, b, a);
}
