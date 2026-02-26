//MRTに出力するための返り値
struct GBufferPSOutput
{
	float4 Albedo : SV_Target0; //アルベド
	float4 Normal : SV_Target1; //法線
	float4 Material : SV_Target2; //マテリアル情報（例：スペキュラ強度、メタリック、粗さなど）
};