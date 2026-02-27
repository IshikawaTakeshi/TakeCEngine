//MRTに出力するための返り値
struct GBufferPSOutput
{
	float4 Albedo : SV_Target0; //アルベド
	float4 Normal : SV_Target1; //法線
	float4 Material : SV_Target2; //マテリアル情報（例：スペキュラ強度、メタリック、粗さなど）
	
	//環境マップテクスチャのSrvインデックス
    int EnvMapTextureIndex : SV_Target3;
};


//---------------------------------------
// 法線のエンコードとデコード
//---------------------------------------
float3 EncodeNormal(float3 normal) {
	return normal * 0.5f + 0.5f;
}

float3 DecodeNormal(float3 encodedNormal) {
	return encodedNormal * 2.0f - 1.0f;
}

//---------------------------------------
// ワールド座標復元
//---------------------------------------
float3 ReconstructWorldPos(float2 uv, float depth01, float4x4 viewProjectionInverse) {
	float2 ndc_xy = uv * 2.0f - 1.0f;
	ndc_xy.y = -ndc_xy.y;
	
	float4 ndc = float4(ndc_xy, depth01, 1.0f);
	float4 wp = mul(ndc, viewProjectionInverse);
	return wp.xyz / wp.w;
}