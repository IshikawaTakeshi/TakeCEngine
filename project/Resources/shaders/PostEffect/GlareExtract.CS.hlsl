
struct GlareExtractInfo {
	float4 threshold;
	float knee;
};

Texture2D gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

ConstantBuffer<GlareExtractInfo> gGlareExtractInfo : register(b0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	int2 uv = int2(DTid.xy);
	int2 dims;
	gInputTexture.GetDimensions(dims.x, dims.y);

    // uv を dst の解像度に合わせて正規化サンプリングする場合は適宜計算する（ここは simplest: 直接 Load）
	if (uv.x >= dims.x || uv.y >= dims.y)
		return;
	float4 c = gInputTexture.Load(int3(uv, 0));

	float L = dot(c.rgb, float3(0.2126, 0.7152, 0.0722));
	float extract = saturate((L - gGlareExtractInfo.threshold) / max(1e-6, L));
	gOutputTexture[uv] = c * extract;
}