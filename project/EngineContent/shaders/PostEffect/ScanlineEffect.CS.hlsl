
//============================================================================
// ScanlineEffect.CS.hlsl
//============================================================================

struct ScanlineParam {
	float intensity;
	float frequency;
	float speed;
	float thickness;
	float time;
	uint isActive;
};

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);

ConstantBuffer<ScanlineParam> gScanlineParam : register(b0);

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID) {
	
	uint w, h;
	gOutputTexture.GetDimensions(w, h);
	if (dtid.x >= w || dtid.y >= h) return;

	float2 uv = (float2(dtid.xy) + 0.5f) / float2(w, h);

	float4 src = gInputTexture[dtid.xy];
	
	 // エフェクト無効ならそのまま出力
	if (!gScanlineParam.isActive) {
		gOutputTexture[dtid.xy] = src;
		return;
	}

    // 走査線マスク作成
	float phase = uv.y * gScanlineParam.frequency + gScanlineParam.time * gScanlineParam.speed;
	float s = 0.5f + 0.5f * sin(phase); // 0..1

    // thicknessでコントラスト調整（簡易）
	float scanMask = smoothstep(
	0.5f - gScanlineParam.thickness * 0.5f, 
	0.5f + gScanlineParam.thickness * 0.5f,
	s
	);

    // 暗線として乗算
	float scan = lerp(1.0f, scanMask, saturate(gScanlineParam.intensity));
	float3 dst = src.rgb * scan;

	gOutputTexture[dtid.xy] = float4(dst, src.a);
}