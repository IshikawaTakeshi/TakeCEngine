#include "Particle.hlsli"
#include "Light/DirectionalLight.hlsli"

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//並行光源
ConstantBuffer<DirectionalLight> gDirLight : register(b1);
//テクスチャ
Texture2D<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;

	float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	float4 baseColor = gMaterial.color * textureColor * input.color;

	// アルファ値が0なら描画しない
	if (output.color.a == 0.0f) {
		discard;
	}
	
	// ライティング適用
	float3 n = normalize(input.normal);
	float3 weights = saturate(float3(
		dot(n, float3(1, 0, 0)),
		dot(n, float3(0, 1, 0)),
		dot(n, float3(0, 0, 1))
	));
	
	
	float3 diffuse =
		input.basisColor0 * weights.x +
		input.basisColor1 * weights.y +
		input.basisColor2 * weights.z;

	float3 colorLit = diffuse;
	output.color = float4(baseColor.rgb * colorLit, baseColor.a);

	output.color.rgb *= baseColor.a;
	return output;
}